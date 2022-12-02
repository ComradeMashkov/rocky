/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#pragma once

#include <rocky/Common.h>
#include <rocky/Status.h>
#include <rocky/StringUtils.h>
#include <list>
#include <istream>
#include <functional>
#include <optional>
#include <nlohmann/json.hpp>

namespace rocky
{
    // New plan??
    //struct Config : public nlohmann::json
    //{
    //};

    class Config;
    using ConfigSet = std::list<Config>;

    /**
     * Config is a general-purpose container for serializable data. You store an object's members
     * to Config, and then translate the Config to a particular format (like XML or JSON). Likewise,
     * the object can de-serialize a Config back into member data. Config support the optional<>
     * template for optional values.
     */
    class ROCKY_EXPORT Config
    {
    public:
        Config()
            : _isLocation(false), _isNumber(false) { }

        Config(const std::string& key)
            : _key(key), _isLocation(false), _isNumber(false) { }

        template<typename T>
        explicit Config(const std::string& key, const T& value)
            : _key(key), _isLocation(false), _isNumber(false)
        {
            setValue(value);
        }

        explicit Config(const std::string& key, const Config& value)
            : _key(key), _isLocation(false), _isNumber(false)
        {
            add(value);
        }

        // Copy CTOR
        Config( const Config& rhs ) :
            _key(rhs._key),
            _defaultValue(rhs._defaultValue),
            _children(rhs._children),
            _referrer(rhs._referrer),
            _isLocation(rhs._isLocation),
            _isNumber(rhs._isNumber),
            _externalRef(rhs._externalRef) 
        {
            //nop
        }

        /**
         * Referrer is the context for resolving relative pathnames that occur in this object.
         * For example, if the value is a filename "file.txt" and the referrer is "C:/temp/a.earth",
         * then the full path of the file is "C:/temp/file.txt".
         *
         * Calling this sets a referrer on this object and its children.
         */
        void setReferrer(const std::string& value);

        /** Access this object's "relative-to" location. */
        const std::string& referrer() const { return _referrer; }

        /** Referrer associated with a key */
        const std::string referrer(const std::string& key) const {
            return child(key).referrer();
        }

        /** Sets whether this Config's value represents a location, i.e. a URI, filename, or
            other string that can be relocated to be relative to a different referrer. */
        void setIsLocation(bool tf) { _isLocation = tf; }
        bool isLocation() const { return _isLocation; }

        /** Hint that this Config came from an externally referenced resource. */
        const std::string& externalRef() const { return _externalRef; }
        void setExternalRef(const std::string& externalRef) { _externalRef = externalRef; }

        /** Create a config object from an XML input stream. */
        bool fromXML(std::istream& in);

        /** Encode this object as JSON. */
        std::string toJSON(bool pretty = false) const;

        /** Populate this object from a JSON string. */
        bool fromJSON(const std::string& json);
        static Config readJSON(const std::string& json);

        /** True if this object contains no data. */
        bool empty() const {
            return _key.empty() && _defaultValue.empty() && _children.empty();
        }

        /** True is this object is a simple key/value pair with no children. */
        bool isSimple() const {
            return !_key.empty() && !_defaultValue.empty() && _children.empty();
        }

        /** The key value for this object */
        std::string& key() { return _key; }
        const std::string& key() const { return _key; }

        /** The value corresponding to the key */
        const std::string& value() const { return _defaultValue; }

        /** Main setValue method - see specializations inline below */
        template<typename T>
        void setValue(const T& value) {
            _defaultValue = util::make_string() << value;
        }

        /** Child objects. */
        ConfigSet& children() { return _children; }
        const ConfigSet& children() const { return _children; }

        /** A collection of all the children of this object with a particular key */
        const ConfigSet children(const std::string& key) const {
            ConfigSet r;
            for (ConfigSet::const_iterator i = _children.begin(); i != _children.end(); i++) {
                if (i->key() == key)
                    r.push_back(*i);
            }
            return r;
        }

        /** Whether this object has a child with a given key */
        bool hasChild(const std::string& key) const {
            for (ConfigSet::const_iterator i = _children.begin(); i != _children.end(); i++)
                if (i->key() == key)
                    return true;
            return false;
        }

        /** Removes all children with the given key */
        void remove(const std::string& key) {
            for (ConfigSet::iterator i = _children.begin(); i != _children.end(); ) {
                if (i->key() == key)
                    i = _children.erase(i);
                else
                    ++i;
            }
        }

        /** First child with the given key */
        const Config& child(const std::string& key) const;

        /** Pointer to the first child with the given key, or NULL if none exist */
        const Config* child_ptr(const std::string& key) const;

        /** Mutable pointer to the first child with the given key, or NULL if none exist */
        Config* mutable_child(const std::string& key);

        /** Merge the contents of another Config object into this object.. danger, read the code
            before you use this */
        void merge(const Config& rhs);

        /** Locate (recursively) the first descendant object with this key, optionally checking
            the current object as well */
        Config* find(const std::string& key, bool checkThis = true);
        const Config* find(const std::string& key, bool checkThis = true) const;

        /** Add a value as a child */
        template<typename T>
        void add(const std::string& key, const T& value) {
            _children.push_back(Config(key, value));
            _children.back().setReferrer(_referrer);
        }

        /** Add a Config as a child */
        void add(const Config& conf) {
            _children.push_back(conf);
            _children.back().setReferrer(_referrer);
        }

        /** Add a config as a child, assigning it a key */
        void add(const std::string& key, const Config& conf) {
            Config temp = conf;
            temp.key() = key;
            add(temp);
        }

        /** Add a set of config objects as children. */
        void add(const ConfigSet& set) {
            for (ConfigSet::const_iterator i = set.begin(); i != set.end(); i++)
                add(*i);
        }

        /** Adds or replaces an optional value as a child, but only if it is set */
        template<typename T>
        void set(const std::string& key, const optional<T>& opt) {
            remove(key);
            if (opt.has_value()) {
                set(Config(key, opt.get()));
            }
        }

        template<typename T>
        void set(const std::string& key, const std::shared_ptr<T>& obj) {
            remove(key);
            if (obj.valid()) {
                Config conf = obj->getConfig();
                conf.key() = key;
                set(conf);
            }
        }

        // If target is set to targetValue, set key to val.
        template<typename X, typename Y>
        void set(const std::string& key, const std::string& val, const optional<X>& target, const Y& targetValue) {
            if (target.isSetTo(targetValue)) {
                remove(key);
                set(key, val);
            }
        }

        /** Adds or replaces a config as a child. */
        void set(const Config& conf) {
            remove(conf.key());
            add(conf);
        }

        /** Sets a key value pair child */
        template<typename T>
        void set(const std::string& key, const T& value) {
            set(Config(key, value));
        }

        /** Whether this object has the key OR has a child with the key */
        bool hasValue(const std::string& key) const {
            return !value(key).empty();
        }

        /** The value of this object (if the key matches) or a matching child object */
        const std::string value(const std::string& key) const {
            std::string r = rocky::util::trim(child(key).value());
            if (r.empty() && _key == key)
                r = _defaultValue;
            return r;
        }

        /** Default value transformed to another type */
        template<typename T>
        T valueAs(const T& fallback) const {
            return rocky::util::as<T>(_defaultValue, fallback);
        }

        /** Value cast to a particular primitive type (with fallback in case casting fails) */
        template<typename T>
        T value(const std::string& key, T fallback) const {
            std::string r;
            if (hasChild(key))
                r = child(key).value();
            return rocky::util::as<T>(r, fallback);
        }

        /** Populates the output value iff the Config exists. */
        template<typename T>
        bool get(const std::string& key, optional<T>& output) const {
            std::string r;
            if (hasChild(key))
                r = child(key).value();
            if (!r.empty()) {
                output = rocky::util::as<T>(r, output.defaultValue());
                return true;
            }
            else
                return false;
        }

        /** Populates the output enumerable pair iff the Config exists. */
        template<typename X, typename Y>
        bool get(const std::string& key, const std::string& val, optional<X>& target, const Y& targetValue) const {
            if (hasValue(key) && value(key) == val) {
                target = targetValue;
                return true;
            }
            else
                return false;
        }

        /** Populates the output enumerable pair iff the Config exists. */
        template<typename X, typename Y>
        bool get(const std::string& key, const std::string& val, X& target, const Y& targetValue) const {
            if (hasValue(key) && value(key) == val) {
                target = targetValue;
                return true;
            }
            return false;
        }

        /** Populates the ouptut value iff the Config exists. */
        template<typename T>
        bool get(const std::string& key, T& output) const {
            if (hasValue(key)) {
                output = value<T>(key, output);
                return true;
            }
            return false;
        }

        // remove everything from (this) that also appears in rhs (diff)
        Config operator - (const Config& rhs) const;

        //! Whether the encoded value is actual a number
        bool isNumber() const { return _isNumber; }

    protected:
        std::string _key;
        std::string _defaultValue;
        ConfigSet _children;
        std::string _referrer;
        bool _isLocation;
        bool _isNumber;
        std::string _externalRef;

    };

    // SPECIALIZATION - Config

    template<> inline
        void Config::set<Config>(const std::string& key, const Config& conf) {
        remove(key);
        Config temp = conf;
        temp.key() = key;
        add(temp);
    }

    template<> inline
        void Config::set<Config>(const std::string& key, const optional<Config>& opt) {
        remove(key);
        if (opt.has_value()) {
            Config conf = opt.value();
            conf.key() = key;
            add(conf);
        }
    }

    template<> inline
        bool Config::get<Config>(const std::string& key, optional<Config>& output) const {
        if (hasChild(key)) {
            output = child(key);
            return true;
        }
        else
            return false;
    }

    // SPECIALIZATIONS - setValue

    template<> inline void Config::setValue<std::string>(const std::string& value) {
        _defaultValue = value;
        _isNumber = false;
    }
    template<> inline void Config::setValue<bool>(const bool& value) {
        _defaultValue = value==true? "true" : "false";
        _isNumber = false;
    }
    template<> inline void Config::setValue<short>(const short& value) {
        _defaultValue = std::to_string(value);
        _isNumber = true;
    }
    template<> inline void Config::setValue<unsigned short>(const unsigned short& value) {
        _defaultValue = std::to_string(value);
        _isNumber = true;
    }
    template<> inline void Config::setValue<int>(const int& value) {
        _defaultValue = std::to_string(value);
        _isNumber = true;
    }
    template<> inline void Config::setValue<unsigned int>(const unsigned int& value) {
        _defaultValue = std::to_string(value);
        _isNumber = true;
    }
    template<> inline void Config::setValue<long>(const long& value) {
        _defaultValue = std::to_string(value);
        _isNumber = true;
    }
    template<> inline void Config::setValue<unsigned long>(const unsigned long& value) {
        _defaultValue = std::to_string(value);
        _isNumber = true;
    }
    template<> inline void Config::setValue<float>(const float& value) {
        _defaultValue = util::Stringify() << std::setprecision(8) << value;
        _isNumber = true;
    }
    template<> inline void Config::setValue<double>(const double& value) {
        _defaultValue = util::Stringify() << std::setprecision(16) << value;
        _isNumber = true;
    }

    template<> inline void Config::set(const std::string& key, const std::vector<std::string>& input) {
        remove(key);
        if (!input.empty()) {
            std::ostringstream buf;
            for(auto i=0; i<input.size(); ++i) {
                if (i > 0) buf << ',';
                bool quote = input[i].find(',') != std::string::npos;
                if (quote) buf << "\"";
                buf << input[i];
                if (quote) buf << "\"";
            }
            set(key, buf.str());
        }
    }
        
    template<> inline bool Config::get(const std::string& key, std::vector<std::string>& output) const {
        if (hasChild(key)) {
            output.clear();
            std::vector<std::string> tokens;
            util::StringTokenizer(value(key), output, ",", "\"", true, true);
            return true;
        }
        else return false;
    }


    // Use this macro to "activate" any object with a getConfig/ctor(const Config&) pair
    // and make it usable with Config::set/get/add.
    // NOTE: You must only use this macro in the global namespace!

#define ROCKY_SPECIALIZE_CONFIG(TYPE) \
    namespace rocky { \
        template<> inline \
        void Config::set<TYPE>(const std::string& key, const TYPE& obj) { \
            set( key, obj.getConfig() ); \
        } \
        template<> inline \
        void Config::set<TYPE>(const std::string& key, const optional<TYPE>& opt) { \
            if ( opt.has_value() ) \
                set(key, opt.value()); \
        } \
        template<> inline \
        bool Config::get<TYPE>(const std::string& key, TYPE& opt) const { \
            if ( hasChild(key) ) { \
                opt = TYPE(child(key)); \
                return true; \
            } \
            else return false; \
        } \
        template<> inline \
        bool Config::get<TYPE>(const std::string& key, optional<TYPE>& opt) const { \
            if ( hasChild(key) ) { \
                opt = TYPE(child(key)); \
                return true; \
            } \
            else return false; \
        } \
        template<> inline void Config::add<TYPE>(const std::string& key, const TYPE& value) { \
            Config conf = value.getConfig(); \
            conf.key() = key; \
            add( conf ); \
        } \
    } // namespace rocky

    //--------------------------------------------------------------------

    /**
     * Base class for all serializable options classes.
     */
    class ROCKY_EXPORT ConfigOptions
    {
    public:
        ConfigOptions(const Config& conf = Config())
            : _conf(conf) { }
        ConfigOptions(const ConfigOptions& rhs)
            : _conf(rhs.getConfig()) { } //rhs._conf ) { }

        virtual ~ConfigOptions();

        const std::string& referrer() const { return _conf.referrer(); }

        ConfigOptions& operator = (const ConfigOptions& rhs) {
            if (this != &rhs) {
                _conf = rhs.getConfig();
                mergeConfig(_conf);
            }
            return *this;
        }

        void merge(const ConfigOptions& rhs) {
            _conf.merge(rhs._conf);
            mergeConfig(rhs.getConfig());
        }

        virtual Config getConfig() const;

        bool empty() const { return _conf.empty(); }

    protected:
        virtual void mergeConfig(const Config& conf) { }

        Config _conf;
    };

    /**
     * Base configoptions class for driver options.
     * @deprecated - will be removed.
     */
    class ROCKY_EXPORT DriverConfigOptions : public ConfigOptions
    {
    public:
        DriverConfigOptions(const ConfigOptions& rhs = ConfigOptions())
            : ConfigOptions(rhs) {
            fromConfig(_conf);
        }

        /** dtor */
        virtual ~DriverConfigOptions();

        /** Gets or sets the name of the driver to load */
        void setDriver(const std::string& value) { _driver = value; }
        const std::string& getDriver() const { return _driver; }

    public:
        virtual Config getConfig() const {
            Config conf = ConfigOptions::getConfig();
            if (!_driver.empty())
                conf.set("driver", _driver);
            return conf;
        }

        virtual void mergeConfig(const Config& conf) {
            ConfigOptions::mergeConfig(conf);
            fromConfig(conf);
        }

    public:
        void fromConfig(const Config& conf) {
            _driver = conf.value("driver");
            if (_driver.empty() && conf.hasValue("type"))
                _driver = conf.value("type");
        }

    private:
        std::string _name, _driver;
    };
}

//! Macro to use when defining a COnfigOptions class
#define META_ConfigOptions(LIBRARY, MYCLASS, SUPERCLASS) \
    protected: \
        virtual void mergeConfig(const Config& conf) { \
            SUPERCLASS ::mergeConfig(conf); \
            fromConfig(conf); \
        } \
    public: \
        MYCLASS () : SUPERCLASS() { fromConfig(_conf); } \
        MYCLASS (const ConfigOptions& opt) : SUPERCLASS(opt) { fromConfig(_conf); }



