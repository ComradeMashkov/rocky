/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#pragma once

#include <rocky_vsg/Common.h>
#include <rocky_vsg/GeometryPool.h>
#include <rocky_vsg/Runtime.h>
#include <rocky_vsg/TerrainState.h>
#include <rocky_vsg/TerrainSettings.h>
#include <rocky_vsg/TerrainTilePager.h>

namespace ROCKY_NAMESPACE
{
    class Map;

    /**
     * Access to all terrain-specific logic, data, and settings
     * associated with a Map.
     */
    class TerrainContext
    {
    public:
        TerrainContext(
            shared_ptr<Map> map,
            const SRS& worldSRS,
            Runtime& runtime,
            const TerrainSettings& settings,
            TerrainTileHost* host);

        //! Terrain settings.
        const TerrainSettings& settings;

        //! runtime operations (scene graph, views, etc)
        Runtime& runtime;

        //! the map this terrain is rendering
        shared_ptr<Map> map;

        //! SRS of the rendered terrain
        SRS worldSRS;

        //! Builds geometry for terrain tiles
        GeometryPool geometryPool;

        //! Tracks and updates state for terrain tiles
        TerrainTilePager tiles;

        //! Creates the state group objects for terrain rendering
        TerrainState stateFactory;

        //! name of job arena used to load data
        std::string loadSchedulerName = "terrain.load";
    };
}
