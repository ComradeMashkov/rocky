/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#include "LocalTangentPlane.h"

using namespace rocky;

#define LC "[LTP] "

// --------------------------------------------------------------------------

TangentPlaneSRS::TangentPlaneSRS(
    const Key& key,
    const dvec3& originLLA) :

    SRS(key),
    _originLLA(originLLA)
{
    _is_user_defined = true;
    _is_ltp = true;
    _domain = PROJECTED;
    _name = "Tangent Plane";

    // set up the LTP matrixes.

    dvec3 xyz = getEllipsoid().geodeticToGeocentric(_originLLA);
    _local2world = getEllipsoid().geocentricToLocalToWorld(xyz);

    //getEllipsoid()->computeLocalToWorldTransformFromLatLongHeight(
    //    deg2rad(_originLLA.y()),
    //    deg2rad(_originLLA.x()),
    //    _originLLA.z(),
    //    _local2world);

    _world2local = glm::inverse(_local2world); //  ::inverse(_local2world);
    //_world2local.invert( _local2world );
}

bool
TangentPlaneSRS::preTransform(
    std::vector<dvec3>& points,
    const SRS** out_srs) const
{
    for(auto& point : points)
    {
        dvec3 world = dvec4(point, 1.0) * _local2world;
        //double lat, lon, height;
        point = getEllipsoid().geocentricToGeodetic(world);
        //getEllipsoid()->convertXYZToLatLongHeight(world.x(), world.y(), world.z(), lat, lon, height);
        //i->x() = osg::rad2deg(lon);
        //i->y() = osg::rad2deg(lat);
        //i->z() = height;
    }

    if (out_srs)
        *out_srs = getGeodeticSRS().get();

    return true;
}

bool
TangentPlaneSRS::postTransform(
    std::vector<dvec3>& points,
    const SRS** out_srs) const
{
    dvec3 world;
    for(auto& point : points)
    {
        world = getEllipsoid().geodeticToGeocentric(point);
        point = dvec4(world, 1.0) * _world2local;
    }

    if (out_srs)
        *out_srs = getGeodeticSRS().get();

    return true;
}

bool
TangentPlaneSRS::_isEquivalentTo(const SRS* srs, bool considerVDatum) const
{
    return
        srs->isLTP() &&
        _originLLA == static_cast<const TangentPlaneSRS*>(srs)->_originLLA;
    // todo: check the reference ellipsoids?
}
