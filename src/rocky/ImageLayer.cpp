/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#pragma once
#include "ImageLayer.h"

#include "Color.h"
#include "ImageMosaic.h"
#include "IOTypes.h"
#include "Metrics.h"
#include "StringUtils.h"
#include "GeoImage.h"
#include "Image.h"
#include "TileKey.h"
#include <cinttypes>

using namespace rocky;
using namespace rocky::util;

#define LC "[ImageLayer] \"" << name().value() << "\" "

// TESTING
//#undef  ROCKY_DEBUG
//#define ROCKY_DEBUG ROCKY_INFO

//------------------------------------------------------------------------
#if 0
void
ImageLayer::Options::fromConfig(const Config& conf)
{
    _transparentColor.setDefault(Color(0, 0, 0, 0));
    //_minFilter.setDefault( osg::Texture::LINEAR_MIPMAP_LINEAR );
    //_magFilter.setDefault( osg::Texture::LINEAR );
    _textureCompression.setDefault("");
    _shared.setDefault( false );
    _coverage.setDefault( false );
    _reprojectedTileSize.setDefault( 256 );

    conf.get( "nodata_image", noDataImageLocation());
    conf.get( "shared", _shared );
    conf.get( "coverage", _coverage );
    conf.get( "altitude", _altitude );
    conf.get( "accept_draping", acceptDraping());
    conf.get( "edge_buffer_ratio", _edgeBufferRatio);
    conf.get( "reprojected_tilesize", _reprojectedTileSize);
    conf.get("transparent_color", transparentColor());

    //if ( conf.hasChild("color_filters") )
    //{
    //    _colorFilters->clear();
    //    ColorFilterRegistry::instance()->readChain( conf.child("color_filters"), _colorFilters.mutable_value() );
    //}

    //conf.get("mag_filter","LINEAR",                _magFilter,osg::Texture::LINEAR);
    //conf.get("mag_filter","LINEAR_MIPMAP_LINEAR",  _magFilter,osg::Texture::LINEAR_MIPMAP_LINEAR);
    //conf.get("mag_filter","LINEAR_MIPMAP_NEAREST", _magFilter,osg::Texture::LINEAR_MIPMAP_NEAREST);
    //conf.get("mag_filter","NEAREST",               _magFilter,osg::Texture::NEAREST);
    //conf.get("mag_filter","NEAREST_MIPMAP_LINEAR", _magFilter,osg::Texture::NEAREST_MIPMAP_LINEAR);
    //conf.get("mag_filter","NEAREST_MIPMAP_NEAREST",_magFilter,osg::Texture::NEAREST_MIPMAP_NEAREST);
    //conf.get("min_filter","LINEAR",                _minFilter,osg::Texture::LINEAR);
    //conf.get("min_filter","LINEAR_MIPMAP_LINEAR",  _minFilter,osg::Texture::LINEAR_MIPMAP_LINEAR);
    //conf.get("min_filter","LINEAR_MIPMAP_NEAREST", _minFilter,osg::Texture::LINEAR_MIPMAP_NEAREST);
    //conf.get("min_filter","NEAREST",               _minFilter,osg::Texture::NEAREST);
    //conf.get("min_filter","NEAREST_MIPMAP_LINEAR", _minFilter,osg::Texture::NEAREST_MIPMAP_LINEAR);
    //conf.get("min_filter","NEAREST_MIPMAP_NEAREST",_minFilter,osg::Texture::NEAREST_MIPMAP_NEAREST);

    conf.get("texture_compression", textureCompression());

    // uniform names
    conf.get("shared_sampler", _shareTexUniformName);
    conf.get("shared_matrix",  _shareTexMatUniformName);

    // automatically set shared=true if the uniform name is set.
    if (shareTexUniformName().has_value() && !shared().has_value())
        shared() = true;
    
    conf.get("async", async());
}

Config
ImageLayer::Options::getConfig() const
{
    Config conf = TileLayer::Options::getConfig();

//    conf.set( "nodata_image",   _noDataImageFilename );
    conf.set( "shared",         _shared );
    conf.set( "coverage",       _coverage );
    conf.set( "altitude",       _altitude );
    conf.set( "accept_draping", acceptDraping());
    conf.set( "edge_buffer_ratio", _edgeBufferRatio);
    conf.set( "reprojected_tilesize", _reprojectedTileSize);
    conf.set("transparent_color", transparentColor());

    //if ( _colorFilters->size() > 0 )
    //{
    //    Config filtersConf("color_filters");
    //    if ( ColorFilterRegistry::instance()->writeChain( _colorFilters.get(), filtersConf ) )
    //    {
    //        conf.set( filtersConf );
    //    }
    //}

    //conf.set("mag_filter","LINEAR",                _magFilter,osg::Texture::LINEAR);
    //conf.set("mag_filter","LINEAR_MIPMAP_LINEAR",  _magFilter,osg::Texture::LINEAR_MIPMAP_LINEAR);
    //conf.set("mag_filter","LINEAR_MIPMAP_NEAREST", _magFilter,osg::Texture::LINEAR_MIPMAP_NEAREST);
    //conf.set("mag_filter","NEAREST",               _magFilter,osg::Texture::NEAREST);
    //conf.set("mag_filter","NEAREST_MIPMAP_LINEAR", _magFilter,osg::Texture::NEAREST_MIPMAP_LINEAR);
    //conf.set("mag_filter","NEAREST_MIPMAP_NEAREST",_magFilter,osg::Texture::NEAREST_MIPMAP_NEAREST);
    //conf.set("min_filter","LINEAR",                _minFilter,osg::Texture::LINEAR);
    //conf.set("min_filter","LINEAR_MIPMAP_LINEAR",  _minFilter,osg::Texture::LINEAR_MIPMAP_LINEAR);
    //conf.set("min_filter","LINEAR_MIPMAP_NEAREST", _minFilter,osg::Texture::LINEAR_MIPMAP_NEAREST);
    //conf.set("min_filter","NEAREST",               _minFilter,osg::Texture::NEAREST);
    //conf.set("min_filter","NEAREST_MIPMAP_LINEAR", _minFilter,osg::Texture::NEAREST_MIPMAP_LINEAR);
    //conf.set("min_filter","NEAREST_MIPMAP_NEAREST",_minFilter,osg::Texture::NEAREST_MIPMAP_NEAREST);

    conf.set("texture_compression", textureCompression());

    // uniform names
    conf.set("shared_sampler", _shareTexUniformName);
    conf.set("shared_matrix",  _shareTexMatUniformName);

    conf.set("async", async());

    return conf;
}
#endif

//------------------------------------------------------------------------

ImageLayer::ImageLayer() :
    super()
{
    construct(Config());
}

ImageLayer::ImageLayer(const Config& conf) :
    super(conf)
{
    construct(conf);
}

void
ImageLayer::construct(const Config& conf)
{
    _transparentColor.setDefault(Color(0, 0, 0, 0));
    //_minFilter.setDefault( osg::Texture::LINEAR_MIPMAP_LINEAR );
    //_magFilter.setDefault( osg::Texture::LINEAR );
    _textureCompression.setDefault("");
    _shared.setDefault(false);
    _coverage.setDefault(false);

    conf.get("nodata_image", _noDataImageLocation);
    conf.get("shared", _shared);
    conf.get("coverage", _coverage);
    conf.get("altitude", _altitude);
    conf.get("accept_draping", _acceptDraping);
    //conf.get("edge_buffer_ratio", _edgeBufferRatio);
    //conf.get("reprojected_tilesize", _reprojectedTileSize);
    conf.get("transparent_color", _transparentColor);

    //if ( conf.hasChild("color_filters") )
    //{
    //    _colorFilters->clear();
    //    ColorFilterRegistry::instance()->readChain( conf.child("color_filters"), _colorFilters.mutable_value() );
    //}

    //conf.get("mag_filter","LINEAR",                _magFilter,osg::Texture::LINEAR);
    //conf.get("mag_filter","LINEAR_MIPMAP_LINEAR",  _magFilter,osg::Texture::LINEAR_MIPMAP_LINEAR);
    //conf.get("mag_filter","LINEAR_MIPMAP_NEAREST", _magFilter,osg::Texture::LINEAR_MIPMAP_NEAREST);
    //conf.get("mag_filter","NEAREST",               _magFilter,osg::Texture::NEAREST);
    //conf.get("mag_filter","NEAREST_MIPMAP_LINEAR", _magFilter,osg::Texture::NEAREST_MIPMAP_LINEAR);
    //conf.get("mag_filter","NEAREST_MIPMAP_NEAREST",_magFilter,osg::Texture::NEAREST_MIPMAP_NEAREST);
    //conf.get("min_filter","LINEAR",                _minFilter,osg::Texture::LINEAR);
    //conf.get("min_filter","LINEAR_MIPMAP_LINEAR",  _minFilter,osg::Texture::LINEAR_MIPMAP_LINEAR);
    //conf.get("min_filter","LINEAR_MIPMAP_NEAREST", _minFilter,osg::Texture::LINEAR_MIPMAP_NEAREST);
    //conf.get("min_filter","NEAREST",               _minFilter,osg::Texture::NEAREST);
    //conf.get("min_filter","NEAREST_MIPMAP_LINEAR", _minFilter,osg::Texture::NEAREST_MIPMAP_LINEAR);
    //conf.get("min_filter","NEAREST_MIPMAP_NEAREST",_minFilter,osg::Texture::NEAREST_MIPMAP_NEAREST);

    conf.get("texture_compression", _textureCompression);

    // uniform names
    //conf.get("shared_sampler", _shareTexUniformName);
    //conf.get("shared_matrix", _shareTexMatUniformName);

    // automatically set shared=true if the uniform name is set.
    //if (_shareTexUniformName.has_value() && !_shared.has_value())
    //    _shared = true;

    conf.get("async", _async);


    //    _useCreateTexture = false;
    _sentry.setName("ImageLayer " + *name());

    // image layers render as a terrain texture.
    //setRenderType(RENDERTYPE_TERRAIN_SURFACE);

#if 0
    if (options().altitude().has_value())
    {
        setAltitude(options().altitude().get());
    }
#endif
    ROCKY_TODO("Altitude");

    if (_acceptDraping.has_value())
    {
        setAcceptDraping(_acceptDraping);
    }
}

Config
ImageLayer::getConfig() const
{
    Config conf = TileLayer::getConfig();

    conf.set("nodata_image", _noDataImageLocation);
    conf.set("shared", _shared);
    conf.set("coverage", _coverage);
    conf.set("altitude", _altitude);
    conf.set("accept_draping", _acceptDraping);
    //conf.get("edge_buffer_ratio", _edgeBufferRatio);
    //conf.get("reprojected_tilesize", _reprojectedTileSize);
    conf.set("transparent_color", _transparentColor);

    //if ( conf.hasChild("color_filters") )
    //{
    //    _colorFilters->clear();
    //    ColorFilterRegistry::instance()->readChain( conf.child("color_filters"), _colorFilters.mutable_value() );
    //}

    //conf.get("mag_filter","LINEAR",                _magFilter,osg::Texture::LINEAR);
    //conf.get("mag_filter","LINEAR_MIPMAP_LINEAR",  _magFilter,osg::Texture::LINEAR_MIPMAP_LINEAR);
    //conf.get("mag_filter","LINEAR_MIPMAP_NEAREST", _magFilter,osg::Texture::LINEAR_MIPMAP_NEAREST);
    //conf.get("mag_filter","NEAREST",               _magFilter,osg::Texture::NEAREST);
    //conf.get("mag_filter","NEAREST_MIPMAP_LINEAR", _magFilter,osg::Texture::NEAREST_MIPMAP_LINEAR);
    //conf.get("mag_filter","NEAREST_MIPMAP_NEAREST",_magFilter,osg::Texture::NEAREST_MIPMAP_NEAREST);
    //conf.get("min_filter","LINEAR",                _minFilter,osg::Texture::LINEAR);
    //conf.get("min_filter","LINEAR_MIPMAP_LINEAR",  _minFilter,osg::Texture::LINEAR_MIPMAP_LINEAR);
    //conf.get("min_filter","LINEAR_MIPMAP_NEAREST", _minFilter,osg::Texture::LINEAR_MIPMAP_NEAREST);
    //conf.get("min_filter","NEAREST",               _minFilter,osg::Texture::NEAREST);
    //conf.get("min_filter","NEAREST_MIPMAP_LINEAR", _minFilter,osg::Texture::NEAREST_MIPMAP_LINEAR);
    //conf.get("min_filter","NEAREST_MIPMAP_NEAREST",_minFilter,osg::Texture::NEAREST_MIPMAP_NEAREST);

    conf.set("texture_compression", _textureCompression);

    return conf;
}

void
ImageLayer::setShared(bool value)
{
    setOptionThatRequiresReopen(_shared, value);
}

bool
ImageLayer::getShared() const
{
    return _shared;
}

void
ImageLayer::setCoverage(bool value)
{
    setOptionThatRequiresReopen(_coverage, value);
}

bool
ImageLayer::getCoverage() const
{
    return _coverage;
}

#if 0
void
ImageLayer::setSharedTextureUniformName(const std::string& value)
{
    if (options().shareTexUniformName() != value)
        options().shareTexUniformName() = value;
}

const std::string&
ImageLayer::getSharedTextureUniformName() const
{
    return options().shareTexUniformName().get();
}

void
ImageLayer::setSharedTextureMatrixUniformName(const std::string& value)
{
    if (options().shareTexMatUniformName() != value)
        options().shareTexMatUniformName() = value;
}

const std::string&
ImageLayer::getSharedTextureMatrixUniformName() const
{
    return options().shareTexMatUniformName().get();
}
#endif

void
ImageLayer::setAsyncLoading(bool value)
{
    _async = value;
}

bool
ImageLayer::getAsyncLoading() const
{
    return _async;
}

//shared_ptr<ImageLayer>
//ImageLayer::create(const ConfigOptions& options)
//{
//    return std::dynamic_pointer_cast<ImageLayer>(Layer::materialize(options));
//}

Status
ImageLayer::openImplementation(const IOOptions* io)
{
    Status parent = TileLayer::openImplementation(io);
    if (parent.failed())
        return parent;

    //if (!_emptyImage.valid())
    //    _emptyImage = ImageUtils::createEmptyImage();

#if 0
    if (!options().shareTexUniformName().has_value())
        options().shareTexUniformName().init("layer_" + std::to_string(getUID()) + "_tex");

    if (!options().shareTexMatUniformName().has_value() )
        options().shareTexMatUniformName().init(options().shareTexUniformName().get() + "_matrix");
#endif

    return Status::NoError;
}


#if 0
void
ImageLayer::setAltitude(const Distance& value)
{
    options().altitude() = value;

    if (value != 0.0)
    {
        osg::StateSet* stateSet = getOrCreateStateSet();

        stateSet->addUniform(
            new osg::Uniform("oe_terrain_altitude", (float)options().altitude()->as(Units::METERS)),
            osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        stateSet->setMode(GL_CULL_FACE, 0);
    }
    else
    {
        osg::StateSet* stateSet = getOrCreateStateSet();
        getOrCreateStateSet()->removeUniform("oe_terrain_altitude");
        stateSet->removeMode(GL_CULL_FACE);
    }
}

const Distance&
ImageLayer::getAltitude() const
{
    return options().altitude().get();
}
#endif

void
ImageLayer::setAcceptDraping(bool value)
{
    _acceptDraping = value;

#if 0
    if (value == true && getStateSet() != nullptr)
        getStateSet()->removeDefine("ROCKY_DISABLE_DRAPING");
    else
        getOrCreateStateSet()->setDefine("ROCKY_DISABLE_DRAPING");
#endif
    ROCKY_TODO("Draping");
}

bool
ImageLayer::getAcceptDraping() const
{
    return _acceptDraping;
}

#if 0
void
ImageLayer::invoke_onCreate(const TileKey& key, GeoImage& data)
{
    if (_callbacks.empty() == false) // not thread-safe but that's ok
    {
        // Copy the vector to prevent thread lockup
        Callbacks temp;

        _callbacks.lock();
        temp = _callbacks;
        _callbacks.unlock();

        for(Callbacks::const_iterator i = temp.begin();
            i != temp.end();
            ++i)
        {
            i->get()->onCreate(key, data);
        }
    }
}

void
ImageLayer::setUseCreateTexture()
{
    _useCreateTexture = true;
}
#endif

Result<GeoImage>
ImageLayer::createImage(
    const TileKey& key) const
{
    return createImage(key, nullptr);
}

Result<GeoImage>
ImageLayer::createImage(
    const TileKey& key,
    IOControl* progress) const
{    
    ROCKY_PROFILING_ZONE;
    ROCKY_PROFILING_ZONE_TEXT(getName() + " " + key.str());

    if (!isOpen())
    {
        return GeoImage::INVALID;
    }

    //NetworkMonitor::ScopedRequestLayer layerRequest(getName());

    GeoImage result = createImageInKeyProfile( key, progress );

#if 0
    // Post-cache operations:
    if (!_postLayers.empty())
    {
        for (auto& post : _postLayers)
        {
            if (!result.valid())
            {
                TileKey bestKey = getBestAvailableTileKey(key);
                result = createImageInKeyProfile(bestKey, progress);
            }

            result = post->createImage(result, key, progress);
        }
    }

    if (result.valid())
    {
        postCreateImageImplementation(result, key, progress);
    }
#endif

    return result;
}

Result<GeoImage>
ImageLayer::createImage(
    const GeoImage& canvas,
    const TileKey& key,
    IOControl* progress)
{
    util::ScopedReadLock lock(layerMutex());
    return createImageImplementation(canvas, key, progress);
}

Result<GeoImage>
ImageLayer::createImageInKeyProfile(
    const TileKey& key,
    IOControl* ioc) const
{
    // If the layer is disabled, bail out.
    if ( !isOpen() )
    {
        return GeoImage::INVALID;
    }

    // Make sure the request is in range.
    // TODO: perhaps this should be a call to mayHaveData(key) instead.
    if ( !isKeyInLegalRange(key) )
    {
        return GeoImage::INVALID;
    }

    // Tile gate prevents two threads from requesting the same key
    // at the same time, which would be unnecessary work. Only lock
    // the gate if there is an L2 cache active
    ROCKY_TODO("Sentry");
    //util::ScopedGate<TileKey> scopedGate(_sentry, key, [&]() {
    //    return _memCache.valid();
    //});

    GeoImage result;

    ROCKY_DEBUG << LC << "create image for \"" << key.str() << "\", ext= "
        << key.getExtent().toString() << std::endl;

    ROCKY_TODO("Caching");
#if 0
    // the cache key combines the Key and the horizontal profile.
    std::string cacheKey = Cache::makeCacheKey(
        strings::Stringify() << key.str() << "-" << std::hex << key.getProfile()->getHorizSignature(),
        "image");

    // The L2 cache key includes the layer revision of course!
    char memCacheKey[64];

    const CachePolicy& policy = ioc->getCacheSettings()->cachePolicy().get();

    // Check the layer L2 cache first
    if ( _memCache.valid() )
    {
        sprintf(memCacheKey, "%d/%s/%s", 
            getRevision(), 
            key.str().c_str(), 
            key.getProfile()->getHorizSignature().c_str());

        CacheBin* bin = _memCache->getOrCreateDefaultBin();
        ReadResult result = bin->readObject(memCacheKey, 0L);
        if (result.succeeded())
        {
            return GeoImage(static_cast<osg::Image*>(result.releaseObject()), key.getExtent());
        }
    }

    // locate the cache bin for the target profile for this layer:
    CacheBin* cacheBin = getCacheBin( key.getProfile() );

    // validate the existance of a valid layer profile (unless we're in cache-only mode, in which
    // case there is no layer profile)
    if ( !policy.isCacheOnly() && !getProfile() )
    {
        disable("Could not establish a valid profile");
        return GeoImage::INVALID;
    }

    osg::ref_ptr< osg::Image > cachedImage;

    // First, attempt to read from the cache. Since the cached data is stored in the
    // map profile, we can try this first.
    if ( cacheBin && policy.isCacheReadable() )
    {
        ReadResult r = cacheBin->readImage(cacheKey, 0L);
        if ( r.succeeded() )
        {
            cachedImage = r.releaseImage();
            bool expired = policy.isExpired(r.lastModifiedTime());
            if (!expired)
            {
                ROCKY_DEBUG << "Got cached image for " << key.str() << std::endl;
                return GeoImage(cachedImage.get(), key.getExtent());
            }
            else
            {
                ROCKY_DEBUG << "Expired image for " << key.str() << std::endl;
            }
        }
    }

    // The data was not in the cache. If we are cache-only, fail sliently
    if ( policy.isCacheOnly() )
    {
        // If it's cache only and we have an expired but cached image, just return it.
        if (cachedImage.valid())
        {
            return GeoImage( cachedImage.get(), key.getExtent() );
        }
        else
        {
            return GeoImage::INVALID;
        }
    }
#endif

    if (key.getProfile()->isHorizEquivalentTo(getProfile()))
    {
        bool createUpsampledImage = false;

        if (upsample() == true &&
            maxDataLevel() > key.getLOD())
        {
            TileKey best = getBestAvailableTileKey(key, false);
            if (best.valid())
            {
                TileKey best_upsampled = getBestAvailableTileKey(key, true);
                if (best_upsampled.valid() &&
                    best.getLOD() < best_upsampled.getLOD())
                {
                    createUpsampledImage = true;
                }
            }
        }

        if (createUpsampledImage == true)
        {
            ROCKY_TODO("upsampled image");
            //result = createFractalUpsampledImage(key, ioc);
        }
        else
        {
            util::ScopedReadLock lock(layerMutex());
            result = createImageImplementation(key, ioc);
        }
    }
    else
    {
        // If the profiles are different, use a compositing method to assemble the tile.
        result = assembleImage(key, ioc);
    }

    // Check for cancelation before writing to a cache:
    if (ioc && ioc->isCanceled())
    {
        return GeoImage::INVALID;
    }

#if 0
    if (result.valid())
    {
        // invoke user callbacks
        invoke_onCreate(key, result);

        if (_memCache.valid())
        {
            CacheBin* bin = _memCache->getOrCreateDefaultBin();
            bin->write(memCacheKey, result.getImage(), 0L);
        }

        // If we got a result, the cache is valid and we are caching in the map profile,
        // write to the map cache.
        if (cacheBin        &&
            policy.isCacheWriteable())
        {
            if ( key.getExtent() != result.getExtent() )
            {
                ROCKY_INFO << LC << "WARNING! mismatched extents." << std::endl;
            }

            cacheBin->write(cacheKey, result.getImage(), 0L);
        }
    }
#endif

#if 0
    else // result.valid() == false
    {
        ROCKY_DEBUG << LC << key.str() << "result INVALID" << std::endl;
        // We couldn't get an image from the source.  So see if we have an expired cached image
        if (cachedImage.valid())
        {
            ROCKY_DEBUG << LC << "Using cached but expired image for " << key.str() << std::endl;
            result = GeoImage( cachedImage.get(), key.getExtent());
        }
    }
#endif

    return result;
}

GeoImage
ImageLayer::assembleImage(
    const TileKey& key,
    IOControl* ioc) const
{
    // If we got here, asset that there's a non-null layer profile.
    if (!getProfile())
    {
        setStatus(Status::Error(Status::AssertionFailure, "assembleImage with undefined profile"));
        return GeoImage::INVALID;
    }

    GeoImage mosaicedImage, result;

#if 0
    // Scale the extent if necessary to apply an "edge buffer"
    GeoExtent ext = key.getExtent();
    if ( options().edgeBufferRatio().has_value() )
    {
        double ratio = options().edgeBufferRatio().get();
        ext.scale(ratio, ratio);
    }
#endif

    // Get a set of layer tiles that intersect the requested extent.
    std::vector<TileKey> intersectingKeys;
    key.getIntersectingKeys(getProfile(), intersectingKeys);
    //getProfile()->getIntersectingTiles( key, intersectingKeys );

    if ( intersectingKeys.size() > 0 )
    {
#if 0
        GeoExtent ee = key.getExtent().transform(intersectingKeys.front().getProfile()->getSRS());
        ROCKY_INFO << "Tile " << key.str() << " ... " << ee.toString() << std::endl;
        for (auto key : intersectingKeys) {
            ROCKY_INFO << " - " << key.str() << " ... " << key.getExtent().toString() << std::endl;
        }
#endif
        double dst_minx, dst_miny, dst_maxx, dst_maxy;
        key.getExtent().getBounds(dst_minx, dst_miny, dst_maxx, dst_maxy);

        // if we find at least one "real" tile in the mosaic, then the whole result tile is
        // "real" (i.e. not a fallback tile)
        bool retry = false;
        util::ImageMosaic mosaic;

        // keep track of failed tiles.
        std::vector<TileKey> failedKeys;

        for(auto& k : intersectingKeys)
        {
            GeoImage image = createImageInKeyProfile(k, ioc);

            if (image.valid())
            {
#if 0
                // use std::dynamic_pointer_cast....
                if (dynamic_cast<const TimeSeriesImage*>(image.getImage()))
                {
                    ROCKY_WARN << LC << "Cannot mosaic a TimeSeriesImage. Discarding." << std::endl;
                    return GeoImage::INVALID;
                }
                else if (dynamic_cast<const osg::ImageStream*>(image.getImage()))
                {
                    ROCKY_WARN << LC << "Cannot mosaic an osg::ImageStream. Discarding." << std::endl;
                    return GeoImage::INVALID;
                }
                else
#endif
                {
                    mosaic.getImages().emplace_back(image.getImage(), k);
                }
            }
            else
            {
                // the tile source did not return a tile, so make a note of it.
                failedKeys.push_back(k);

                if (ioc && ioc->isCanceled())
                {
                    retry = true;
                    break;
                }
            }
        }

        // Fail is: a) we got no data and the LOD is greater than zero; or
        // b) the operation was canceled mid-stream.
        if ( (mosaic.getImages().empty() && key.getLOD() > 0) || retry)
        {
            // if we didn't get any data at LOD>0, fail.
            ROCKY_DEBUG << LC << "Couldn't create image for ImageMosaic " << std::endl;
            return GeoImage::INVALID;
        }

        // We got at least one good tile, OR we got nothing but since the LOD==0 we have to
        // fall back on a lower resolution.
        // So now we go through the failed keys and try to fall back on lower resolution data
        // to fill in the gaps. The entire mosaic must be populated or this qualifies as a bad tile.
        for(auto& k : failedKeys)
        {
            GeoImage geoimage;

            for(TileKey parentKey = k.createParentKey();
                parentKey.valid() && !geoimage.valid();
                parentKey.makeParent())
            {
                {
                    util::ScopedReadLock lock(layerMutex());
                    geoimage = createImageImplementation(parentKey, ioc);
                }

                if (geoimage.valid())
                {
                    GeoImage cropped;

                    if ( !isCoverage() )
                    {
                        cropped = geoimage.crop(
                            k.getExtent(),
                            false,
                            geoimage.getImage()->width(),
                            geoimage.getImage()->height() );
                    }

                    else
                    {
                        // TODO: may not work.... test; tilekey extent will <> cropped extent
                        cropped = geoimage.crop(
                            k.getExtent(),
                            true,
                            geoimage.getImage()->width(),
                            geoimage.getImage()->height(),
                            false );
                    }

                    // and queue it.
                    mosaic.getImages().emplace_back(cropped.getImage(), k);

                }
            }

            if (!geoimage.valid())
            {
                // a tile completely failed, even with fallback. Eject.
                ROCKY_DEBUG << LC << "Couldn't fallback on tiles for ImageMosaic" << std::endl;
                // let it go. The empty areas will be filled with alpha by ImageMosaic.
            }
        }

        // all set. Mosaic all the images together.
        double rxmin, rymin, rxmax, rymax;
        mosaic.getExtents( rxmin, rymin, rxmax, rymax );

        mosaicedImage = GeoImage(
            mosaic.createImage(),
            GeoExtent( getProfile()->getSRS(), rxmin, rymin, rxmax, rymax ) );
    }
    else
    {
        ROCKY_DEBUG << LC << "assembleImage: no intersections (" << key.str() << ")" << std::endl;
    }

    // Final step: transform the mosaic into the requesting key's extent.
    if ( mosaicedImage.valid() )
    {
        // GeoImage::reproject() will automatically crop the image to the correct extents.
        // so there is no need to crop after reprojection. Also note that if the SRS's are the
        // same (even though extents are different), then this operation is technically not a
        // reprojection but merely a resampling.

        const GeoExtent& extent = key.getExtent();

        result = mosaicedImage.reproject(
            key.getProfile()->getSRS(),
            &extent,
            tileSize(), tileSize(),
            true);
    }

    if (ioc && ioc->isCanceled())
    {
        return GeoImage::INVALID;
    }

    return result;
}

Status
ImageLayer::writeImage(const TileKey& key, const Image* image, IOControl* progress)
{
    if (getStatus().failed())
        return getStatus();

    util::ScopedReadLock lock(layerMutex());
    return writeImageImplementation(key, image, progress);
}

Status
ImageLayer::writeImageImplementation(const TileKey& key, const Image* image, IOControl* progress) const
{
    return Status(Status::ServiceUnavailable);
}

const std::string
ImageLayer::getCompressionMethod() const
{
    if (isCoverage())
        return "none";

    return _textureCompression;
}

void
ImageLayer::modifyTileBoundingBox(const TileKey& key, Box& box) const
{
    if (_altitude.has_value())
    {
        if (_altitude->as(Units::METERS) > box.zmax)
        {
            box.zmax = _altitude->as(Units::METERS);
        }
    }
    TileLayer::modifyTileBoundingBox(key, box);
}

#if 0
void
ImageLayer::addCallback(ImageLayer::Callback* c)
{
    _callbacks.lock();
    _callbacks.push_back(c);
    _callbacks.unlock();
}

void
ImageLayer::removeCallback(ImageLayer::Callback* c)
{
    _callbacks.lock();
    Callbacks::iterator i = std::find(_callbacks.begin(), _callbacks.end(), c);
    if (i != _callbacks.end())
        _callbacks.erase(i);
    _callbacks.unlock();
}
#endif

#if 0
void
ImageLayer::addPostLayer(ImageLayer* layer)
{
    util::ScopedMutexLock lock(_postLayers);
    _postLayers.push_back(layer);
}
#endif

//...................................................................

#if 0
#define ARENA_ASYNC_LAYER "oe.layer.async"
//#define FUTURE_IMAGE_COLOR_PLACEHOLDER

FutureTexture2D::FutureTexture2D(
    ImageLayer* layer,
    const TileKey& key) :

    osg::Texture2D(),
    FutureTexture(),
    _layer(layer),
    _key(key)
{
    // since we'll be updating it mid stream
    setDataVariance(osg::Object::DYNAMIC);

    setName(_key.str() + ":" + _layer->getName());

    // start loading the image
    dispatch();
}

void
FutureTexture2D::dispatch() const
{
    osg::observer_ptr<ImageLayer> layer_ptr(_layer);
    TileKey key(_key);

    Job job(JobArena::get(ARENA_ASYNC_LAYER));
    job.setName(Stringify() << key.str() << " " << _layer->getName());

    // prioritize higher LOD tiles.
    job.setPriority(key.getLOD());

    _result = job.dispatch<GeoImage>(
        [layer_ptr, key](Cancelable* progress) mutable
        {
            GeoImage result;
            osg::ref_ptr<ImageLayer> safe(layer_ptr);
            if (safe.valid())
            {
                osg::ref_ptr<ProgressCallback> p = new ProgressCallback(progress);
                result = safe->createImage(key, p.get());
            }
            return result;
        });
}

void
FutureTexture2D::update()
{
    if (_resolved)
    {
        return;
    }

    else if (_result.isCanceled())
    {
        dispatch();
        return;
    }

    else if (_result.isAvailable() == true)
    {
        ROCKY_DEBUG<< LC << "Async result available for " << getName() << std::endl;

        // fetch the result
        GeoImage geoImage = _result.get();

        if (geoImage.getStatus().failed())
        {
            ROCKY_DEBUG << LC << "Error: " << geoImage.getStatus().message() << std::endl;
            _failed = true;
        }
        else
        {
            osg::ref_ptr<osg::Image> image = geoImage.takeImage();

            if (image.valid())
            {
                this->setImage(image);
                this->dirtyTextureObject();
            }

            else
            {
                _failed = true;
                this->dirtyTextureObject();
            }
        }

        // reset the future so update won't be called again
        _result.abandon();

        _resolved = true;
    }
}
#endif

#if 0
GeoImage
ImageLayer::createFractalUpsampledImage(
    const TileKey& key,
    ProgressCallback* progress)
{
    ROCKY_PROFILING_ZONE;

    // Input metatile grid. Always use the immediate parent for
    // fractal enhancement.
    MetaTile<GeoImage> input;
    input.setCreateTileFunction(
        [&](const TileKey& key, ProgressCallback* p) -> GeoImage
        {
            if (this->isKeyInLegalRange(key))
                return this->createImage(key, p);
            else
                return GeoImage::INVALID;
        }
    );
    TileKey parentKey = key.createParentKey();
    dmat4 scale_bias;
    key.getExtent().createScaleBias(parentKey.getExtent(), scale_bias);
    input.setCenterTileKey(parentKey, scale_bias);

    // validate that we have a good metatile.
    if (input.valid() == false)
        return GeoImage::INVALID;

    // set up a workspace for creating the new image.
    int ws_width = getTileSize() + 3;
    int ws_height = getTileSize() + 3;

    osg::ref_ptr<osg::Image> workspace = new osg::Image();
    workspace->allocateImage(
        ws_width, ws_height, 1,
        input.getCenterTile().getImage()->getPixelFormat(),
        input.getCenterTile().getImage()->getDataType(),
        input.getCenterTile().getImage()->getPacking());

    ImageUtils::PixelWriter writeToWorkspace(workspace.get());
    ImageUtils::PixelReader readFromWorkspace(workspace.get());

    // output image:
    osg::ref_ptr<osg::Image> output = new osg::Image();
    output->allocateImage(
        getTileSize(), getTileSize(), 1,
        input.getCenterTile().getImage()->getPixelFormat(),
        input.getCenterTile().getImage()->getDataType(),
        input.getCenterTile().getImage()->getPacking());

    // Random number generator for fractal algorithm:
    Util::Random prng(key.hash());

    // temporaries:
    GeoImage::pixel_type pixel, p0, p1, p2, p3;
    float k0, k1, k2, k3;
    unsigned r;
    int s, t;

    // First pass: loop over the grid and populate even-numbered
    // pixels with values from the ancestors.
    for (t = 0; t < ws_height; t += 2)
    {
        for (s = 0; s < ws_width; s += 2)
        {
            input.read(pixel, s - 2, t - 2);
            writeToWorkspace(pixel, s, t);

            if (progress && progress->isCanceled())
                return GeoImage::INVALID;
        }

        if (progress && progress->isCanceled())
            return GeoImage::INVALID;
    }

    // Second pass: diamond
    for (t = 1; t < workspace->t() - 1; t += 2)
    {
        for (s = 1; s < workspace->s() - 1; s += 2)
        {
            r = prng.next(4u);

            // Diamond: pick one of the four diagonals to copy into the
            // center pixel, attempting to preserve curves. When there is
            // no clear choice, go random.
            readFromWorkspace(p0, s - 1, t - 1); k0 = p0.r();
            readFromWorkspace(p1, s + 1, t - 1); k1 = p1.r();
            readFromWorkspace(p2, s + 1, t + 1); k2 = p2.r();
            readFromWorkspace(p3, s - 1, t + 1); k3 = p3.r();

            // three the same
            if (k0 == k1 && k1 == k2 && k2 != k3) pixel = p0;
            else if (k1 == k2 && k2 == k3 && k3 != k0) pixel = p1;
            else if (k2 == k3 && k3 == k0 && k0 != k1) pixel = p2;
            else if (k3 == k0 && k0 == k1 && k1 != k2) pixel = p3;

            // continuations
            else if (k0 == k2 && k0 != k1 && k0 != k3) pixel = p0;
            else if (k1 == k3 && k1 != k2 && k1 != k0) pixel = p1;

            // all else, random.
            else pixel = (r == 0) ? p0 : (r == 1) ? p1 : (r == 2) ? p2 : p3;

            writeToWorkspace(pixel, s, t);
        }
    }

    // Third pass: square
    for (t = 2; t < workspace->t() - 1; ++t)
    {
        for (s = 2; s < workspace->s() - 1; ++s)
        {
            if (((s & 1) == 1 && (t & 1) == 0) || ((s & 1) == 0 && (t & 1) == 1))
            {
                r = prng.next(4u);

                // Square: pick one of the four adjacents to copy into the
                // center pixel, attempting to preserve curves. When there is
                // no clear choice, go random.
                readFromWorkspace(p0, s - 1, t); k0 = p0.r();
                readFromWorkspace(p1, s, t - 1); k1 = p1.r();
                readFromWorkspace(p2, s + 1, t); k2 = p2.r();
                readFromWorkspace(p3, s, t + 1); k3 = p3.r();

                // three the same
                if (k0 == k1 && k1 == k2 && k2 != k3) pixel = p0;
                else if (k1 == k2 && k2 == k3 && k3 != k0) pixel = p1;
                else if (k2 == k3 && k3 == k0 && k0 != k1) pixel = p2;
                else if (k3 == k0 && k0 == k1 && k1 != k2) pixel = p3;

                // continuations
                else if (k0 == k2 && k0 != k1 && k0 != k3) pixel = p0;
                else if (k1 == k3 && k1 != k2 && k1 != k0) pixel = p1;

                // all else, random.
                else pixel = (r == 0) ? p0 : (r == 1) ? p1 : (r == 2) ? p2 : p3;

                writeToWorkspace(pixel, s, t);
            }
        }
    }

    // finally blit from workspace interior to the output image.
    ImageUtils::PixelWriter writeToOutput(output.get());
    for (t = 0; t < output->t(); ++t)
    {
        for (s = 0; s < output->s(); ++s)
        {
            readFromWorkspace(pixel, s + 2, t + 2);
            writeToOutput(pixel, s, t);
        }
    }

    if (progress && progress->isCanceled())
    {
        return GeoImage::INVALID;
    }

    return GeoImage(output.get(), key.getExtent());
}
#endif
