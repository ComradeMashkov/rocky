/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#include "TileKey.h"
#include "Math.h"
#include "GeoPoint.h"

using namespace rocky;

TileKey TileKey::INVALID( 0, 0, 0, nullptr );

TileKey::TileKey(
    unsigned int lod, unsigned int tile_x, unsigned int tile_y,
    Profile::ptr profile)
{
    _x = tile_x;
    _y = tile_y;
    _lod = lod;
    _profile = profile;
    rehash();
}

void
TileKey::rehash()
{
    _hash = valid() ?
        rocky::hash_value_unsigned(
            (std::size_t)_lod, 
            (std::size_t)_x,
            (std::size_t)_y,
            _profile->hash()) :
        0ULL;
}

Profile::ptr
TileKey::getProfile() const
{
    return _profile;
}

const GeoExtent
TileKey::getExtent() const
{
    if (!valid())
        return GeoExtent::INVALID;

    double width, height;
    _profile->getTileDimensions(_lod, width, height);
    double xmin = _profile->getExtent().xMin() + (width * (double)_x);
    double ymax = _profile->getExtent().yMax() - (height * (double)_y);
    double xmax = xmin + width;
    double ymin = ymax - height;

    return GeoExtent( _profile->getSRS(), xmin, ymin, xmax, ymax );
}

const std::string
TileKey::str() const
{
    if (valid())
    {
        return
            std::to_string(_lod) + '/' +
            std::to_string(_x) + '/' +
            std::to_string(_y);
    }
    else return "invalid";
}

unsigned
TileKey::getQuadrant() const
{
    if ( _lod == 0 )
        return 0;
    bool xeven = (_x & 1) == 0;
    bool yeven = (_y & 1) == 0;
    return 
        xeven && yeven ? 0 :
        xeven          ? 2 :
        yeven          ? 1 : 3;
}

std::pair<double, double>
TileKey::getResolution(unsigned tileSize) const
{
    double width, height;
    _profile->getTileDimensions(_lod, width, height);
    return std::make_pair(
        width/(double)(tileSize-1),
        height/(double)(tileSize-1));
}

TileKey
TileKey::createChildKey( unsigned int quadrant ) const
{
    unsigned int lod = _lod + 1;
    unsigned int x = _x * 2;
    unsigned int y = _y * 2;

    if (quadrant == 1)
    {
        x+=1;
    }
    else if (quadrant == 2)
    {
        y+=1;
    }
    else if (quadrant == 3)
    {
        x+=1;
        y+=1;
    }
    return TileKey(lod, x, y, _profile);
}


TileKey
TileKey::createParentKey() const
{
    if (_lod == 0) return TileKey::INVALID;

    unsigned int lod = _lod - 1;
    unsigned int x = _x / 2;
    unsigned int y = _y / 2;
    return TileKey(lod, x, y, _profile);
}

bool
TileKey::makeParent()
{
    if (_lod == 0)
    {
        _profile = NULL; // invalidate
        return false;
    }

    _lod--;
    _x >>= 1;
    _y >>= 1;
    rehash();
    return true;
}

TileKey
TileKey::createAncestorKey(unsigned ancestorLod) const
{
    if (ancestorLod > _lod)
        return TileKey::INVALID;

    unsigned x = _x, y = _y;
    for (unsigned i = _lod; i > ancestorLod; i--)
    {
        x /= 2;
        y /= 2;
    }
    return TileKey(ancestorLod, x, y, _profile);
}

TileKey
TileKey::createNeighborKey( int xoffset, int yoffset ) const
{
    ROCKY_SOFT_ASSERT_AND_RETURN(valid(), TileKey::INVALID);

    unsigned tx, ty;
    getProfile()->getNumTiles( _lod, tx, ty );

    int sx = (int)_x + xoffset;
    unsigned x =
        sx < 0        ? (unsigned)((int)tx + sx) :
        sx >= (int)tx ? (unsigned)sx - tx :
        (unsigned)sx;

    int sy = (int)_y + yoffset;
    unsigned y =
        sy < 0        ? (unsigned)((int)ty + sy) :
        sy >= (int)ty ? (unsigned)sy - ty :
        (unsigned)sy;

    //ROCKY_NOTICE << "Returning neighbor " << x << ", " << y << " for tile " << str() << " offset=" << xoffset << ", " << yoffset << std::endl;

    return TileKey(_lod, x % tx, y % ty, _profile);
}

TileKey
TileKey::mapResolution(unsigned targetSize,
                       unsigned sourceSize,
                       unsigned minimumLOD) const
{
    // This only works when falling back; i.e. targetSize is smaller than sourceSize.
    if ( getLOD() == 0 || targetSize >= sourceSize )
        return *this;

    // Minimum target tile size.
    if ( targetSize < 2 )
        targetSize = 2;

    int lod = (int)getLOD();
    int targetSizePOT = nextPowerOf2((int)targetSize);

    while(true)
    {
        if (targetSizePOT >= (int)sourceSize)
        {
            return createAncestorKey(lod);
        }

        if ( lod == (int)minimumLOD )
        {
            return createAncestorKey(lod);
        }

        lod--;
        targetSizePOT *= 2;
    }
}



TileKey
TileKey::createTileKeyContainingPoint(
    double x, double y, unsigned level,
    shared_ptr<Profile> profile)
{
    ROCKY_SOFT_ASSERT_AND_RETURN(profile && profile->valid(), TileKey::INVALID);

    auto& extent = profile->getExtent();

    if (extent.contains(x, y))
    {
        unsigned tilesX, tilesY;
        profile->getNumTiles(level, tilesX, tilesY);
        //unsigned tilesX = _numTilesWideAtLod0 * (1 << (unsigned)level);
        //unsigned tilesY = _numTilesHighAtLod0 * (1 << (unsigned)level);

        // overflow checks:
#if 0
        if (_numTilesWideAtLod0 == 0u || ((tilesX / _numTilesWideAtLod0) != (1 << (unsigned)level)))
            return TileKey::INVALID;

        if (_numTilesHighAtLod0 == 0u || ((tilesY / _numTilesHighAtLod0) != (1 << (unsigned)level)))
            return TileKey::INVALID;
#endif

        double rx = (x - extent.xMin()) / extent.width();
        int tileX = std::min((unsigned)(rx * (double)tilesX), tilesX - 1);
        double ry = (y - extent.yMin()) / extent.height();
        int tileY = std::min((unsigned)((1.0 - ry) * (double)tilesY), tilesY - 1);

        return TileKey(level, tileX, tileY, profile);
    }
    else
    {
        return TileKey::INVALID;
    }
}

TileKey
TileKey::createTileKeyContainingPoint(
    const GeoPoint& point,
    unsigned level,
    shared_ptr<Profile> profile)
{
    ROCKY_SOFT_ASSERT_AND_RETURN(point.valid() && profile, TileKey::INVALID);

    if (point.getSRS()->isHorizEquivalentTo(profile->getSRS().get()))
    {
        return createTileKeyContainingPoint(
            point.x(), point.y(), level, profile);
    }
    else
    {
        return createTileKeyContainingPoint(
            point.transform(profile->getSRS()), level, profile);
    }
}

namespace
{
    void addIntersectingKeys(
        const GeoExtent& key_ext,
        unsigned localLOD,
        shared_ptr<Profile> target_profile,
        std::vector<TileKey>& out_intersectingKeys)
    {
        ROCKY_SOFT_ASSERT_AND_RETURN(
            !key_ext.crossesAntimeridian(),
            void(),
            "addIntersectingTiles cannot process date-line cross");

        int tileMinX, tileMaxX;
        int tileMinY, tileMaxY;

        double destTileWidth, destTileHeight;
        target_profile->getTileDimensions(localLOD, destTileWidth, destTileHeight);

        auto profile_extent = target_profile->getExtent();

        double west = key_ext.xMin() - profile_extent.xMin();
        double east = key_ext.xMax() - profile_extent.xMin();
        double south = profile_extent.yMax() - key_ext.yMin();
        double north = profile_extent.yMax() - key_ext.yMax();

        tileMinX = (int)(west / destTileWidth);
        tileMaxX = (int)(east / destTileWidth);

        tileMinY = (int)(north / destTileHeight);
        tileMaxY = (int)(south / destTileHeight);

        // If the east or west border fell right on a tile boundary
        // but doesn't actually use that tile, detect that and eliminate
        // the extranous tiles. (This happens commonly when mapping
        // geodetic to mercator for example)

        double quantized_west = destTileWidth * (double)tileMinX;
        double quantized_east = destTileWidth * (double)(tileMaxX + 1);

        if (equivalent(west - quantized_west, destTileWidth))
            ++tileMinX;
        if (equivalent(quantized_east - east, destTileWidth))
            --tileMaxX;

        if (tileMaxX < tileMinX)
            tileMaxX = tileMinX;

        unsigned int numWide, numHigh;
        target_profile->getNumTiles(localLOD, numWide, numHigh);

        // bail out if the tiles are out of bounds.
        if (tileMinX >= (int)numWide || tileMinY >= (int)numHigh ||
            tileMaxX < 0 || tileMaxY < 0)
        {
            return;
        }

        tileMinX = clamp(tileMinX, 0, (int)numWide - 1);
        tileMaxX = clamp(tileMaxX, 0, (int)numWide - 1);
        tileMinY = clamp(tileMinY, 0, (int)numHigh - 1);
        tileMaxY = clamp(tileMaxY, 0, (int)numHigh - 1);

        ROCKY_DEBUG << std::fixed << "  Dest Tiles: " << tileMinX << "," << tileMinY << " => " << tileMaxX << "," << tileMaxY << std::endl;

        for (int i = tileMinX; i <= tileMaxX; ++i)
        {
            for (int j = tileMinY; j <= tileMaxY; ++j)
            {
                //TODO: does not support multi-face destination keys.
                out_intersectingKeys.push_back(TileKey(localLOD, i, j, target_profile));
            }
        }
    }
}

void
TileKey::getIntersectingKeys(
    shared_ptr<Profile> target_profile,
    std::vector<TileKey>& out_intersectingKeys) const
{
    ROCKY_SOFT_ASSERT_AND_RETURN(valid(), void());

    //If the profiles are exactly equal, just add the given tile key.
    if (getProfile()->isHorizEquivalentTo(target_profile))
    {
        //Clear the incoming list
        out_intersectingKeys.clear();
        out_intersectingKeys.push_back(*this);
    }
    else
    {
        // figure out which LOD in the local profile is a best match for the LOD
        // in the source LOD in terms of resolution.
        unsigned target_LOD = target_profile->getEquivalentLOD(getProfile(), getLOD());
        getIntersectingKeys(getExtent(), target_LOD, target_profile, out_intersectingKeys);
        //ROCKY_DEBUG << LC << "GIT, key=" << key.str() << ", localLOD=" << localLOD
        //    << ", resulted in " << out_intersectingKeys.size() << " tiles" << std::endl;
    }
}

void
TileKey::getIntersectingKeys(
    const GeoExtent& input,
    unsigned localLOD,
    shared_ptr<Profile> target_profile,
    std::vector<TileKey>& out_intersectingKeys)
{
    ROCKY_SOFT_ASSERT_AND_RETURN(input.valid() && target_profile && target_profile->valid(), void());

    std::vector<GeoExtent> target_extents;

    target_profile->transformAndExtractContiguousExtents(
        input,
        target_extents);

    for (auto& extent : target_extents)
    {
        addIntersectingKeys(extent, localLOD, target_profile, out_intersectingKeys);
    }

#if 0
    // reproject into the profile's SRS if necessary:
    if (!target_profile->getSRS()->isHorizEquivalentTo(extent.getSRS().get()))
    {
        // localize the extents and clamp them to legal values
        target_extent = target_profile->clampAndTransformExtent(extent);
        if (!target_extent.valid())
            return;
    }

    if (e.crossesAntimeridian())
    {
        GeoExtent first, second;
        if (e.splitAcrossAntimeridian(first, second))
        {
            addIntersectingKeys(first, localLOD, target_profile, out_intersectingKeys);
            addIntersectingKeys(second, localLOD, target_profile, out_intersectingKeys);
        }
    }
    else
    {
        addIntersectingKeys(e, localLOD, target_profile, out_intersectingKeys);
    }
#endif
}