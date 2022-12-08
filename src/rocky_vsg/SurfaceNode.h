/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#pragma once

#include <rocky_vsg/Common.h>
#include <rocky/Image.h>
#include <rocky/SRS.h>
#include <rocky/TileKey.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/vk/State.h>
#include <vsg/maths/vec3.h>

namespace rocky
{
    class Heightfield;
    class Horizon;

    class HorizonTileCuller
    {
    public:
        // four points representing to upper face of the bounding box
        dvec3 _points[4];

        // Horizon object to consider for culling
        shared_ptr<Horizon> _horizon;

        //! Reconfigure the culler
        void set(shared_ptr<SRS> srs, const vsg::dmat4& local2world, const vsg::dbox& bbox);

        //! True if this tile may be visible relative to the horizon
        bool isVisible(const dvec3& from) const;
    };


    /**
     * SurfaceNode holds the geometry and transform information
     * for one terrain tile surface.
     */
    class SurfaceNode :
        public vsg::Inherit<vsg::MatrixTransform, SurfaceNode>
    {
    public:
        SurfaceNode(
            const TileKey& tilekey);

        //! Update the elevation raster associated with this tile
        void setElevation(
            shared_ptr<Image> raster,
            const dmat4& scaleBias);

        //! Elevation raster representing this surface
        shared_ptr<Image> getElevationRaster() const {
            return _elevationRaster;
        }

        //! Elevation matrix representing to this surface
        const dmat4& getElevationMatrix() const {
            return _elevationMatrix;
        }

        //! aligned BBOX for culling (derived from elevation raster)
        //const vsg::box& getAlignedBoundingBox() const {
        //    return _drawable->bound;
        //}
        
        //! Horizon visibility check
        inline bool isVisibleFrom(vsg::State* state) const {
            auto eye = state->modelviewMatrixStack.top() * vsg::dvec3(0, 0, 0);
            return _horizonCuller.isVisible(dvec3(eye.x, eye.y, eye.z));
        }

        //inline bool isVisibleFrom(const fvec3& viewpoint) const {
        //    return _horizonCuller.isVisible(viewpoint);
        //}
     
#if 0
        // A box can have 4 children. 
        // Returns true if any child box intersects the sphere of radius centered around point
        inline bool anyChildBoxIntersectsSphere(const vsg::dvec3& point, float radiusSquared) {
            for(int c=0; c<4; ++c) {
                for(int j=0; j<8; ++j) {
                    if ( length2(_childrenCorners[c][j]-point) < radiusSquared )
                        return true;
                }
            }
            return false;
        }
#endif

        bool anyChildBoxWithinRange(float range, vsg::State* state) const
        {
            for (unsigned i = 0u; i < 32u; ++i) {
                auto d = state->lodDistance(_spheres[i]);
                if (d >= 0.0 && d <= range)
                    return true;
            }
            return false;
        }

        void setLastFramePassedCull(unsigned fn);

        unsigned getLastFramePassedCull() const { return _lastFramePassedCull; }

        void recomputeBound();

        //float getPixelSizeOnScreen(osg::CullStack* cull) const;

        vsg::dsphere boundingSphere;

    protected:

        TileKey _tileKey;
        static const bool _enableDebugNodes;
        int _lastFramePassedCull;
        HorizonTileCuller _horizonCuller;
        shared_ptr<Image> _elevationRaster;
        dmat4 _elevationMatrix;

        using VectorPoints = vsg::dvec3[8];
        using ChildrenCorners = VectorPoints[4];
        ChildrenCorners _childrenCorners;
        vsg::sphere _spheres[32];
        vsg::dbox _localbbox;
        void recomputeLocalBBox();

        std::vector<vsg::vec3> _proxyMesh;
    };

}