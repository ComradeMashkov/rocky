/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#include "GeometryPool.h"
#include "TerrainSettings.h"
#include <rocky/Notify.h>
//#include <vsg/commands/BindVertexBuffers.h>
//#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/DrawIndexed.h>

#undef LC
#define LC "[GeometryPool] "

using namespace rocky;

GeometryPool::GeometryPool() :
    _enabled(true),
    _debug(false),
    _mutex("GeometryPool"),
    _keygate("GeometryPool.keygate")
{
    //ROCKY_TODO("ADJUST_UPDATE_TRAV_COUNT(this, +1)");

    // activate debugging mode
    if ( getenv("ROCKY_DEBUG_REX_GEOMETRY_POOL") != 0L )
    {
        _debug = true;
    }

    if ( ::getenv("ROCKY_REX_NO_POOL") )
    {
        _enabled = false;
        ROCKY_INFO << LC << "Geometry pool disabled (environment)" << std::endl;
    }
}

vsg::ref_ptr<SharedGeometry>
GeometryPool::getPooledGeometry(
    const TileKey& tileKey,
    const Map* map,
    const TerrainSettings& settings,
    Cancelable* progress)
{
    vsg::ref_ptr<SharedGeometry> out;

    // convert to a unique-geometry key:
    GeometryKey geomKey;
    createKeyForTileKey( tileKey, settings.tileSize, geomKey );

    // make our globally shared EBO if we need it
    {
        util::ScopedLock lock(_mutex);
        if (_defaultIndices == nullptr)
        {
            _defaultIndices = createIndices(settings);
        }
    }

    ROCKY_TODO("MeshEditor meshEditor(tileKey, tileSize, map, nullptr);");

    if ( _enabled )
    {
        // Protect access on a per key basis to prevent the same key from being created twice.  
        // This was causing crashes with multiple windows opening and closing.
        util::ScopedGate<GeometryKey> gatelock(_keygate, geomKey);

        // first check the sharing cache:
        //if (!meshEditor.hasEdits())
        {
            util::ScopedLock lock(_mutex);
            auto i = _sharedGeometries.find(geomKey);
            if (i != _sharedGeometries.end())
            {
                // found it:
                out = i->second.get();
            }
        }

        if (!out.valid())
        {
            out = createGeometry(
                tileKey,
                settings,
                //meshEditor,
                progress);

            // only store as a shared geometry if there are no constraints.
            if (out.valid()) //&& !meshEditor.hasEdits())
            {
                util::ScopedLock lock(_mutex);
                _sharedGeometries[geomKey] = out.get();
            }
        }
    }

    else
    {
        out = createGeometry(
            tileKey,
            settings,
            //meshEditor,
            progress);
    }

    return out;
}

void
GeometryPool::createKeyForTileKey(
    const TileKey& tileKey,
    unsigned tileSize,
    GeometryKey& out) const
{
    out.lod  = tileKey.getLOD();
    out.tileY = tileKey.getProfile()->getSRS()->isGeographic()? tileKey.getTileY() : 0;
    out.size = tileSize;
}

int
GeometryPool::getNumSkirtElements(
    unsigned tileSize,
    float skirtRatio) const
{
    return skirtRatio > 0.0f ? (tileSize-1) * 4 * 6 : 0;
}

namespace
{
    int getMorphNeighborIndexOffset(unsigned col, unsigned row, int rowSize)
    {
        if ( (col & 0x1)==1 && (row & 0x1)==1 ) return rowSize+2;
        if ( (row & 0x1)==1 )                   return rowSize+1;
        if ( (col & 0x1)==1 )                   return 2;
        return 1;
    }
}

#define addSkirtDataForIndex(P, INDEX, HEIGHT) \
{ \
    verts->set(P, (*verts)[INDEX] ); \
    normals->set(P, (*normals)[INDEX] ); \
    uvs->set(P, (*uvs)[INDEX] ); \
    uvs->at(P).z = (float)((int)uvs->at(P).z | VERTEX_SKIRT); \
    if ( neighbors ) neighbors->set(P, (*neighbors)[INDEX] ); \
    if ( neighborNormals ) neighborNormals->set(P, (*neighborNormals)[INDEX] ); \
    ++P; \
    verts->set(P, (*verts)[INDEX] - ((*normals)[INDEX])*(HEIGHT) ); \
    normals->set(P, (*normals)[INDEX] ); \
    uvs->set(P, (*uvs)[INDEX] ); \
    uvs->at(P).z = (float)((int)uvs->at(P).z | VERTEX_SKIRT); \
    if ( neighbors ) neighbors->set(P, (*neighbors)[INDEX] - ((*normals)[INDEX])*(HEIGHT) ); \
    if ( neighborNormals ) neighborNormals->set(P, (*neighborNormals)[INDEX] ); \
    ++P; \
}

#define addSkirtTriangles(P, INDEX0, INDEX1) \
{ \
    indices->set(P++, (INDEX0));   \
    indices->set(P++, (INDEX0)+1); \
    indices->set(P++, (INDEX1));   \
    indices->set(P++, (INDEX1));   \
    indices->set(P++, (INDEX0)+1); \
    indices->set(P++, (INDEX1)+1); \
}

vsg::ref_ptr<vsg::ushortArray>
GeometryPool::createIndices(
    const TerrainSettings& settings) const
{
    ROCKY_HARD_ASSERT(settings.tileSize > 0u);

    // Attempt to calculate the number of verts in the surface geometry.
    bool needsSkirt = settings.skirtRatio > 0.0f;
    uint32_t tileSize = settings.tileSize;

    unsigned numVertsInSurface = (tileSize*tileSize);
    unsigned numVertsInSkirt = needsSkirt ? (tileSize - 1) * 2u * 4u : 0;
    unsigned numVerts = numVertsInSurface + numVertsInSkirt;
    unsigned numIndicesInSurface = (tileSize - 1) * (tileSize - 1) * 6;
    unsigned numIncidesInSkirt = getNumSkirtElements(tileSize, settings.skirtRatio);
    unsigned numIndices = numIndicesInSurface + numIncidesInSkirt;

    auto indices = vsg::ushortArray::create(numIndices);

    // tessellate the surface:
    unsigned p = 0;
    for (unsigned j = 0; j < tileSize - 1; ++j)
    {
        for (unsigned i = 0; i < tileSize - 1; ++i)
        {
            int i00 = j * tileSize + i;
            int i01 = i00 + tileSize;
            int i10 = i00 + 1;
            int i11 = i01 + 1;

            unsigned k = j * tileSize + i;

            indices->set(p++, i01);
            indices->set(p++, i00);
            indices->set(p++, i11);

            indices->set(p++, i00);
            indices->set(p++, i10);
            indices->set(p++, i11);
        }
    }

    if (needsSkirt)
    {
        // add the elements for the skirt:
        int skirtBegin = numVertsInSurface;
        int skirtEnd = skirtBegin + numVertsInSkirt;
        int i;
        for (i = skirtBegin; i < (int)skirtEnd - 3; i += 2)
        {
            addSkirtTriangles(p, i, i + 2);
        }
        addSkirtTriangles(p, i, skirtBegin);
    }

    return indices;
}

namespace
{
    struct Locator
    {
        dmat4 _xform;
        bool _isGeographic;
        const Ellipsoid& _ellipsoid;
        //dmat4 _inverse{ 1 };

        Locator(const GeoExtent& extent) :
            _isGeographic(extent.getSRS()->isGeographic()),
            _ellipsoid(extent.getSRS()->getEllipsoid())
        {
            //_xform = glm::translate(dmat4(1), dvec3(extent.xmin(), extent.ymin(), 0));
            //_xform = glm::scale(_xform, dvec3(extent.width(), extent.height(), 1));

            _xform = dmat4(
                extent.width(), 0.0, 0.0, 0.0,
                0.0, extent.height(), 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                extent.xMin(), extent.yMin(), 0.0, 1.0);

            //_inverse = glm::inverse(_xform);
        }

        //void worldToUnit(const dvec3& world, dvec3& unit) const {
        //    if (_srs->isGeographic())
        //        unit = _inverse * _srs->getEllipsoid().geocentricToGeodetic(world);
        //    else
        //        unit = _inverse * world;
        //}

        inline dvec3 unitToWorld(const dvec3& unit) const {
            dvec3 world = _xform * unit;
            if (_isGeographic)
                world = _ellipsoid.geodeticToGeocentric(world);
            return world;
        }
    };
}

vsg::ref_ptr<SharedGeometry>
GeometryPool::createGeometry(
    const TileKey& tileKey,
    const TerrainSettings& settings,
    //MeshEditor& editor,
    Cancelable* progress) const
{
    // Establish a local reference frame for the tile:
    dvec3 centerWorld;
    GeoPoint centroid = tileKey.getExtent().getCentroid();
    centroid.toWorld( centerWorld );

    dmat4 world2local;
    centroid.createWorldToLocal(world2local);

    // Attempt to calculate the number of verts in the surface geometry.
    bool needsSkirt = settings.skirtRatio > 0.0f;

    auto tileSize = settings.tileSize;
    const uint32_t numVertsInSurface    = (tileSize*tileSize);
    const uint32_t numVertsInSkirt      = needsSkirt ? (tileSize-1)*2u * 4u : 0;
    const uint32_t numVerts             = numVertsInSurface + numVertsInSkirt;
    const uint32_t numIndiciesInSurface = (tileSize-1) * (tileSize-1) * 6;
    const uint32_t numIncidesInSkirt    = getNumSkirtElements(tileSize, settings.skirtRatio);

    ROCKY_TODO("GLenum mode = gpuTessellation ? GL_PATCHES : GL_TRIANGLES;");

    Sphere tileBound;

    // the initial vertex locations:
    auto verts = vsg::vec3Array::create(numVerts);
    auto normals = vsg::vec3Array::create(numVerts);
    auto uvs = vsg::vec3Array::create(numVerts);
    vsg::ref_ptr<vsg::vec3Array> neighbors;
    vsg::ref_ptr<vsg::vec3Array> neighborNormals;

    if (settings.morphTerrain == true)
    {
        neighbors = vsg::vec3Array::create(numVerts);
        neighborNormals = vsg::vec3Array::create(numVerts);
    }

#if 0
    if (editor.hasEdits())
    {
        bool tileHasData = editor.createTileMesh(
            geom.get(),
            tileSize,
            skirtRatio,
            mode,
            progress);

        if (geom->empty())
            return nullptr;
    }

    else // default mesh - no constraintsv
#endif
    {
        dvec3 unit;
        dvec3 world;
        dvec3 local;
        dvec3 world_plus_one;
        dvec3 normal;

        Locator locator(tileKey.getExtent());

        for (unsigned row = 0; row < tileSize; ++row)
        {
            float ny = (float)row / (float)(tileSize - 1);
            for (unsigned col = 0; col < tileSize; ++col)
            {
                float nx = (float)col / (float)(tileSize - 1);
                unsigned i = row * tileSize + col;

                unit = dvec3(nx, ny, 0.0);
                world = locator.unitToWorld(unit);
                local = world2local * world;
                verts->set(i, vsg::vec3(local.x, local.y, local.z));

                tileBound.expandBy(local);

                // Use the Z coord as a type marker
                float marker = VERTEX_VISIBLE;
                uvs->set(i, vsg::vec3(nx, ny, marker));

                unit.z = 1.0;
                world_plus_one = locator.unitToWorld(unit);
                normal = glm::normalize((world2local*world_plus_one) - local);
                normals->set(i, vsg::vec3(normal.x, normal.y, normal.z));

                // neighbor:
                if (neighbors)
                {
                    auto& modelNeighborLTP = (*verts)[verts->size() - getMorphNeighborIndexOffset(col, row, tileSize)];
                    neighbors->set(i, modelNeighborLTP);
                }

                if (neighborNormals)
                {
                    auto& modelNeighborNormalLTP = (*normals)[normals->size() - getMorphNeighborIndexOffset(col, row, tileSize)];
                    neighborNormals->set(i, modelNeighborNormalLTP);
                }
            }
        }

        if (needsSkirt)
        {
            // calculate the skirt extrusion height
            float height = (float)tileBound.radius * settings.skirtRatio;

            // Normal tile skirt first:
            unsigned skirtIndex = verts->size();
            unsigned p = skirtIndex;

            // first, create all the skirt verts, normals, and texcoords.
            for (int c = 0; c < (int)tileSize - 1; ++c)
                addSkirtDataForIndex(p, c, height); //south

            for (int r = 0; r < (int)tileSize - 1; ++r)
                addSkirtDataForIndex(p, r*tileSize + (tileSize - 1), height); //east

            for (int c = tileSize - 1; c > 0; --c)
                addSkirtDataForIndex(p, (tileSize - 1)*tileSize + c, height); //north

            for (int r = tileSize - 1; r > 0; --r)
                addSkirtDataForIndex(p, r*tileSize, height); //west
        }

        auto indices =
            _enabled ? _defaultIndices : createIndices(settings);

        // the geometry:
        auto geom = SharedGeometry::create();

        geom->assignArrays(vsg::DataList{
            verts, normals, uvs, neighbors, neighborNormals });

        geom->assignIndices(indices);

        geom->commands.push_back(
            vsg::DrawIndexed::create(
                indices->size(), // index count
                1,               // instance count
                0,               // first index
                0,               // vertex offset
                0));             // first instance

        // maintain for calculating proxy geometries
        geom->proxy_verts = verts;
        geom->proxy_normals = normals;
        geom->proxy_uvs = uvs;
        geom->proxy_indices = indices;

        return geom;
    }
}

#if 0
void
GeometryPool::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() == nv.UPDATE_VISITOR && _enabled)
    {
        Threading::ScopedMutexLock lock(_geometryMapMutex);

        std::vector<GeometryKey> keys;

        for(auto& iter : _geometryMap)
        {
            if (iter.second.get()->referenceCount() == 1)
            {
                keys.push_back(iter.first);
                //iter.second->releaseGLObjects(nullptr);
                OE_DEBUG << "Releasing: " << iter.second.get() << std::endl;
            }
        }

        for(auto& key : keys)
        {
            _geometryMap.erase(key);
        }
    }

    osg::Group::traverse(nv);
}
#endif

void
GeometryPool::clear()
{
    //releaseGLObjects(nullptr);
    util::ScopedLock lock(_mutex);
    _sharedGeometries.clear();
}

#if 0
void
GeometryPool::resizeGLObjectBuffers(unsigned maxsize)
{
    if (!_enabled)
        return;

    // collect all objects in a thread safe manner
    util::ScopedLock lock(_geometryMapMutex);

    for (GeometryMap::const_iterator i = _geometryMap.begin(); i != _geometryMap.end(); ++i)
    {
        i->second->resizeGLObjectBuffers(maxsize);
    }

    // the shared primitive set
    if (_defaultPrimSet.valid())
    {
        _defaultPrimSet->resizeGLObjectBuffers(maxsize);
    }
}

void
GeometryPool::releaseGLObjects(osg::State* state) const
{
    if (!_enabled)
        return;

    // collect all objects in a thread safe manner
    util::ScopedLock lock(_geometryMapMutex);

    for (auto& entry : _geometryMap)
    {
        entry.second->releaseGLObjects(state);
    }

    // the shared primitive set
    if (_defaultPrimSet.valid())
    {
        _defaultPrimSet->releaseGLObjects(state);
    }
}
#endif