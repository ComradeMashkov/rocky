/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#include "Layer.h"
#include "GeoExtent.h"

using namespace ROCKY_NAMESPACE;

#define LC "[Layer] \"" << getName() << "\" "

//.................................................................

#if 0
Config
Layer::Options::getConfig() const
{
    Config conf = ConfigOptions::getConfig();
    conf.set("name", name());
    //conf.set("enabled", enabled());
    conf.set("open", openAutomatically());
    conf.set("enabled", openAutomatically()); // back compat
    conf.set("cacheid", cacheId());
    conf.set("cachepolicy", cachePolicy());
    conf.set("shader_define", shaderDefine());
    conf.set("attribution", attribution());
    conf.set("terrain", terrainPatch());
    conf.set("proxy", _proxySettings );
    conf.set("osg_options", osgOptionString());
    conf.set("l2_cache_size", l2CacheSize());

    //for(std::vector<ShaderOptions>::const_iterator i = shaders().begin();
    //    i != shaders().end();
    //    ++i)
    //{
    //    conf.add("shader", i->getConfig());
    //}

    return conf;
}

void
Layer::Options::fromConfig(const Config& conf)
{
    // defaults:
    openAutomatically().setDefault(true);
    terrainPatch().setDefault(false);

    conf.get("name", name());
    conf.get("open", openAutomatically()); // back compat
    conf.get("enabled", openAutomatically());
    conf.get("cache_id", cacheId()); // compat
    conf.get("cacheid", cacheId());
    conf.get("attribution", attribution());
    conf.get("cache_policy", cachePolicy());
    conf.get("l2_cache_size", l2CacheSize());

    // legacy support:
    if (!cachePolicy().has_value())
    {
        if ( conf.value<bool>( "cache_only", false ) == true )
            cachePolicy()->usage = CachePolicy::USAGE_CACHE_ONLY;
        if ( conf.value<bool>( "cache_enabled", true ) == false )
            cachePolicy()->usage = CachePolicy::USAGE_NO_CACHE;
        if (conf.value<bool>("caching", true) == false)
            cachePolicy()->usage = CachePolicy::USAGE_NO_CACHE;
    }

    //conf.get("shader_define", shaderDefine());
    //const ConfigSet& shadersConf = conf.children("shader");
    //for(ConfigSet::const_iterator i = shadersConf.begin(); i != shadersConf.end(); ++i)
    //    shaders().push_back(ShaderOptions(*i));

    conf.get("terrain", terrainPatch());
    conf.get("patch", terrainPatch());
    conf.get("proxy", _proxySettings );
    conf.get("osg_options", osgOptionString());
}

//.................................................................
#endif

//void
//Layer::TraversalCallback::traverse(osg::Node* node, osg::NodeVisitor* nv) const
//{
//    node->accept(*nv);
//}

//.................................................................

Layer::Layer() :
    super()
{
    construct(Config());
}

Layer::Layer(const Config& conf) :
    super()
{
    construct(conf);
}

void
Layer::construct(const Config& conf)
{
    _revision = 1;
    _uid = rocky::createUID();
    _isClosing = false;
    _isOpening = false;
    _reopenRequired = false;
    _renderType = RENDERTYPE_NONE;

    _openAutomatically.setDefault(true);
    _l2cachesize.setDefault(0);
    _cachePolicy.setDefault(CachePolicy::DEFAULT);

    conf.get("name", _name);
    conf.get("open", _openAutomatically);
    conf.get("cacheid", _cacheid);
    conf.get("cachepolicy", _cachePolicy);
    conf.get("attribution", _attribution);
    conf.get("l2_cache_size", _l2cachesize);

    _status = Status(
        Status::ResourceUnavailable,
        getOpenAutomatically() ? "Layer closed" : "Layer disabled");

#if 0
    // For detecting scene graph changes at runtime
    _sceneGraphCallbacks = new SceneGraphCallbacks(this);

    // Copy the layer options name into the Object name.
    // This happens here AND in open.
    if (options().name().has_value())
    {
        osg::Object::setName(options().name().get());
    }
    else
    {
        osg::Object::setName("Unnamed " + std::string(className()));
    }
#endif

//    _mutex = new util::ReadWriteMutex(name());
}

Config
Layer::getConfig() const
{
    Config conf;
    conf.set("name", _name);
    //conf.set("enabled", enabled());
    conf.set("open", _openAutomatically);
    conf.set("cacheid", _cacheid);
    conf.set("cachepolicy", _cachePolicy);
    //conf.set("shader_define", shaderDefine());
    conf.set("attribution", _attribution);
    //conf.set("terrain", terrainPatch());
    //conf.set("proxy", _proxySettings);
    //conf.set("osg_options", osgOptionString());
    conf.set("l2_cache_size", _l2cachesize);
    return conf;
}

Layer::~Layer()
{
    //nop
}

void
Layer::dirty()
{
    bumpRevision();
}

void
Layer::bumpRevision()
{
    ++_revision;
}

#if 0
void
Layer::setReadOptions(const osgDB::Options* readOptions)
{
    // We are storing _cacheSettings both in the Read Options AND
    // as a class member. This is probably not strictly necessary
    // but we will keep the ref in the Layer just to be on the safe
    // side - gw

    _readOptions = Registry::cloneOrCreateOptions(readOptions);

    // store the referrer for relative-path resolution
    URIContext(options().referrer()).store(_readOptions.get());

    //Store the proxy settings in the options structure.
    if (options().proxySettings().has_value())
    {
        options().proxySettings()->apply(_readOptions.get());
    }

    if (options().osgOptionString().has_value())
    {
        _readOptions->setOptionString(
            options().osgOptionString().get() + " " +
            _readOptions->getOptionString());
    }
}

const osgDB::Options*
Layer::getReadOptions() const
{
    return _readOptions.get();
}
#endif

void
Layer::removeCallback(UID uid)
{
    onLayerOpened.functions.erase(uid);
    onLayerClosed.functions.erase(uid);
}

bool
Layer::getOpenAutomatically() const
{
    return _openAutomatically.value();
}

void
Layer::setOpenAutomatically(bool value)
{
    _openAutomatically = value;
}

const Status&
Layer::setStatus(const Status& status) const
{
    _status = status;
    return _status;
}

const Status&
Layer::setStatus(const Status::Code& code, const std::string& message) const
{
    return setStatus(Status(code, message));
}

void
Layer::setCachePolicy(const CachePolicy& value)
{
    _cachePolicy = value;

#if 0
    if (_cacheSettings.valid())
    {
        _cacheSettings->integrateCachePolicy(options().cachePolicy());
    }
#endif
}

const CachePolicy&
Layer::cachePolicy() const
{
    return _cachePolicy.value();
    //return options().cachePolicy().get();
}

Status
Layer::open(const IOOptions& io)
{
    // Cannot open a layer that's already open OR is disabled.
    if (isOpen())
    {
        return status();
    }

    std::unique_lock lock(_mutex);

    // be optimistic :)
    _status = StatusOK;

#if 0
    // Install any shader #defines
    if (options().shaderDefine().has_value() && !options().shaderDefine()->empty())
    {
        ROCKY_INFO << LC << "Setting shader define " << options().shaderDefine().get() << "\n";
        getOrCreateStateSet()->setDefine(options().shaderDefine().get());
    }
#endif

    _isOpening = true;
    setStatus(openImplementation(io));
    if (isOpen())
    {
        //fireCallback(&LayerCallback::onOpen);
    }
    _isOpening = false;

    return status();
}

Status
Layer::open()
{
    return open(IOOptions());
}

Status
Layer::openImplementation(const IOOptions& io)
{
#if 0
    // Create some local cache settings for this layer.
    // There might be a CacheSettings object in the readoptions that
    // came from the map. If so, copy it.
    CacheSettings* oldSettings = CacheSettings::get(_readOptions.get());
    _cacheSettings = oldSettings ? new CacheSettings(*oldSettings) : new CacheSettings();

    // If the layer hints are set, integrate that cache policy next.
    _cacheSettings->integrateCachePolicy(layerHints().cachePolicy());

    // bring in the new policy for this layer if there is one:
    _cacheSettings->integrateCachePolicy(options().cachePolicy());

    // if caching is a go, install a bin.
    if (_cacheSettings->isCacheEnabled())
    {
        _runtimeCacheId = getCacheID();

        // make our cacheing bin!
        CacheBin* bin = _cacheSettings->getCache()->addBin(_runtimeCacheId);
        if (bin)
        {
            ROCKY_INFO << LC << "Cache bin is [" << _runtimeCacheId << "]\n";
            _cacheSettings->setCacheBin(bin);
        }
        else
        {
            // failed to create the bin, so fall back on no cache mode.
            ROCKY_WARN << LC << "Failed to open a cache bin [" << _runtimeCacheId << "], disabling caching\n";
            _cacheSettings->cachePolicy() = CachePolicy::NO_CACHE;
        }
    }

    // Store it for further propagation!
    _cacheSettings->store(_readOptions.get());
#endif
    return StatusOK;
}

Status
Layer::closeImplementation()
{
    return status();
}

Status
Layer::close()
{    
    if (isOpen())
    {
        std::unique_lock lock(_mutex);
        _isClosing = true;
        closeImplementation();
        _status = Status(Status::ResourceUnavailable, "Layer closed");
        _runtimeCacheId = "";
        //fireCallback(&LayerCallback::onClose);
        _isClosing = false;
    }
    return status();
}

bool
Layer::isOpen() const
{
    return status().ok();
}

const Status&
Layer::status() const
{
    return _status;
}

#if 0
void
Layer::invoke_prepareForRendering(TerrainEngine* engine)
{
    prepareForRendering(engine);

    // deprecation path; call this in case some older layer is still
    // implementing it.
    setTerrainResources(engine->getResources());
}

void
Layer::prepareForRendering(TerrainEngine* engine)
{
    // Install an earth-file shader if necessary (once)
    for (const auto& shaderOptions : options().shaders())
    {
        LayerShader* shader = new LayerShader(shaderOptions);
        shader->install(this, engine->getResources());
        _shaders.emplace_back(shader);
    }
}
#endif

const char*
Layer::typeName() const
{
    return typeid(*this).name();
}

std::shared_ptr<Layer>
Layer::materialize(const ConfigOptions& options)
{
    ROCKY_TODO("New version of old Layer::create");
    return nullptr;
}

#if 0
#define LAYER_OPTIONS_TAG "rocky.LayerOptions"

Layer*
Layer::create(const ConfigOptions& options)
{
    std::string name = options.getConfig().key();

    if (name.empty())
        name = options.getConfig().value("driver");

    if ( name.empty() )
    {
        // fail silently
        ROCKY_DEBUG << "[Layer] ILLEGAL- Layer::create requires a valid driver name" << std::endl;
        return nullptr;
    }

    // convey the configuration options:
    osg::ref_ptr<osgDB::Options> dbopt = Registry::instance()->cloneOrCreateOptions();
    dbopt->setPluginData( LAYER_OPTIONS_TAG, (void*)&options );

    //std::string pluginExtension = std::string( ".osgearth_" ) + name;
    std::string pluginExtension = std::string( "." ) + name;

    // use this instead of osgDB::readObjectFile b/c the latter prints a warning msg.
    osgDB::ReaderWriter::ReadResult rr = osgDB::Registry::instance()->readObject( pluginExtension, dbopt.get() );
    if ( !rr.validObject() || rr.error() )
    {
        // quietly fail so we don't get tons of msgs.
        return 0L;
    }

    Layer* layer = dynamic_cast<Layer*>( rr.getObject() );
    if ( layer == 0L )
    {
        // TODO: communicate an error somehow
        return 0L;
    }

    if (layer->getName().empty())
        layer->setName(name);

    rr.takeObject();
    return layer;
}

const ConfigOptions&
Layer::getConfigOptions(const osgDB::Options* options)
{
    static ConfigOptions s_default;
    const void* data = options->getPluginData(LAYER_OPTIONS_TAG);
    return data ? *static_cast<const ConfigOptions*>(data) : s_default;
}
#endif

#if 0
SceneGraphCallbacks*
Layer::getSceneGraphCallbacks() const
{
    return _sceneGraphCallbacks.get();
}
#endif

#if 0
void
Layer::addCallback(LayerCallback* cb)
{
    _callbacks.push_back( cb );
}

void
Layer::removeCallback(LayerCallback* cb)
{
    CallbackVector::iterator i = std::find( _callbacks.begin(), _callbacks.end(), cb );
    if ( i != _callbacks.end() )
        _callbacks.erase( i );
}
#endif

#if 0
void
Layer::apply(osg::Node* node, osg::NodeVisitor* nv) const
{
    if (_traversalCallback.valid())
    {
        _traversalCallback->operator()(node, nv);
    }
    else
    {
        node->accept(*nv);
    }
}

void
Layer::setCullCallback(TraversalCallback* cb)
{
    _traversalCallback = cb;
}

const Layer::TraversalCallback*
Layer::getCullCallback() const
{
    return _traversalCallback.get();
}
#endif

const GeoExtent&
Layer::extent() const
{
    static GeoExtent s_invalid;
    return s_invalid;
}

DateTimeExtent
Layer::dateTimeExtent() const
{
    return DateTimeExtent();
}

#if 0
osg::StateSet*
Layer::getOrCreateStateSet()
{
    if (!_stateSet.valid())
    {
        _stateSet = new osg::StateSet();
        _stateSet->setName("Layer");
    }
    return _stateSet.get();
}

osg::StateSet*
Layer::getStateSet() const
{
    return _stateSet.get();
}
#endif

#if 0
void
Layer::fireCallback(LayerCallback::MethodPtr method)
{
    for (CallbackVector::iterator i = _callbacks.begin(); i != _callbacks.end(); ++i)
    {
        LayerCallback* cb = dynamic_cast<LayerCallback*>(i->get());
        if (cb) (cb->*method)(this);
    }
}
#endif

std::string
Layer::attribution() const
{
    // Get the attribution from the layer if it's set.
    return _attribution;
}

void
Layer::setAttribution(const std::string& value)
{
    _attribution = value;
}

void
Layer::modifyTileBoundingBox(const TileKey& key, const Box& box) const
{
    //NOP
}

const Layer::Hints&
Layer::hints() const
{
    return _hints;
}
