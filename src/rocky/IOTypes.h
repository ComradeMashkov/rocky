/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#pragma once

#include <rocky/Config.h>
#include <rocky/DateTime.h>
#include <rocky/Status.h>
//#include <rocky/Containers.h>
#include <rocky/Units.h>

/**
 * A collection of types used by the various I/O systems.
 */
namespace rocky
{
    class ROCKY_EXPORT IOOptions
    {
    };

    class Cancelable
    {
    public:
        virtual bool isCanceled() const = 0;
    };

    class ROCKY_EXPORT IOControl : public IOOptions, public Cancelable
    {
    public:
        //TODO: implement
        bool isCanceled() const {
            return _canceled;
        }

    public: // Cancelable
        void cancel() {
            _canceled = true;
        }

        //TODO - stack class only?
        //Should replace Progress?
        //Holds a pointer to io options?
    private:
        bool _canceled = false;
    };

    class ROCKY_EXPORT CachePolicy
    {
    public:
        enum Usage
        {
            USAGE_READ_WRITE = 0,  // read/write to the cache if one exists.
            USAGE_CACHE_ONLY = 1,  // treat the cache as the ONLY source of data.
            USAGE_READ_ONLY = 2,  // read from the cache, but don't write new data to it.
            USAGE_NO_CACHE = 3   // neither read from or write to the cache
        };

        optional<Usage> usage;
        optional<Duration> maxAge;
        optional<DateTime> minTime;


        //! default cache policy (READ_WRITE)
        static CachePolicy DEFAULT;

        //! policy indicating to never use a cache
        static CachePolicy NO_CACHE;

        //! policy indicating to only use a cache
        static CachePolicy CACHE_ONLY;

    public:
        //! constructs an invalid CachePolicy
        CachePolicy();

        //! constructs a CachePolicy from a usage
        CachePolicy(const Usage&);

        //! constructs a CachePolicy from a config options
        CachePolicy(const Config& conf);

        //! Merges any set properties in another CP into this one, override existing values.
        void mergeAndOverride(const CachePolicy& rhs);
        void mergeAndOverride(const optional<CachePolicy>& rhs);

        //! Gets the oldest timestamp for which to accept a cache record
        DateTime getMinAcceptTime() const;

        //! Whether the given timestamp is considered to be expired based on this CachePolicy
        bool isExpired(TimeStamp lastModified) const;

    public: // convenience functions.

        bool isCacheEnabled() const {
            return isCacheReadable() || isCacheWriteable();
        }

        bool isCacheDisabled() const {
            return !isCacheEnabled();
        }

        bool isCacheReadable() const {
            return
                usage.value() == USAGE_READ_WRITE ||
                usage.value() == USAGE_CACHE_ONLY ||
                usage.value() == USAGE_READ_ONLY;
        }

        bool isCacheWriteable() const {
            return usage.value() == USAGE_READ_WRITE;
        }

        bool isCacheOnly() const {
            return usage.value() == USAGE_CACHE_ONLY;
        }

        bool operator == (const CachePolicy& rhs) const;

        bool operator != (const CachePolicy& rhs) const {
            return !operator==(rhs);
        }

        CachePolicy& operator = (const CachePolicy& rhs);

        // returns a readable string describing usage
        std::string usageString() const;

    public: // config
        Config getConfig() const;
        void fromConfig(const Config& conf);
    };

    /**
    * Proxy server configuration.
    */
    class ROCKY_EXPORT ProxySettings
    {
    public:
        std::string hostname;
        int port = -1;
        std::string username;
        std::string password;

        ProxySettings(const Config& conf = Config());
        Config getConfig() const;
    };

    using Headers = std::unordered_map<std::string,std::string>;

    /**
     * Convenience metadata tags
     */
    struct ROCKY_EXPORT IOMetadata
    {
        static const std::string CONTENT_TYPE;
    };

    /**
     * Return value from a read* method
     */
    template<typename T>
    struct IOResult : public Result<T>
    {
        /** Read result codes. */
        enum IOCode
        {
            RESULT_OK,
            RESULT_CANCELED,
            RESULT_NOT_FOUND,
            RESULT_EXPIRED,
            RESULT_SERVER_ERROR,
            RESULT_TIMEOUT,
            RESULT_NO_READER,
            RESULT_READER_ERROR,
            RESULT_UNKNOWN_ERROR,
            RESULT_NOT_IMPLEMENTED,
            RESULT_NOT_MODIFIED
        };

        unsigned ioCode = RESULT_OK;
        TimeStamp lastModifiedTime = 0;
        Duration duration;
        bool fromCache = false;
        Config metadata;

        template<typename T>
        IOResult(const T& result) :
            Result<T>(result) { }

        /** Construct a result with an error message */
        IOResult(const Status& s) :
            Result<T>(s),
            ioCode(s.ok() ? RESULT_OK : RESULT_NOT_FOUND) { }

        template<typename RHS>
        static IOResult<T> propagate(const IOResult<RHS>& rhs) {
            IOResult<T> lhs((Status)rhs);
            lhs.ioCode = rhs.ioCode;
            lhs.metadata = rhs.metadata;
            lhs.fromCache = rhs.fromCache;
            lhs.lastModifiedTime = rhs.lastModifiedTime;
            lhs.duration = rhs.duration;
            return lhs;
        }

        ///** Whether the read operation succeeded */
        //bool succeeded() const { Result<T>::ok(); }

        ///** Whether the read operation failed */
        //bool failed() const { return !succeeded(); }

        /** Gets a string describing the read result */
        static std::string getResultCodeString(unsigned code)
        {
            return
                code == RESULT_OK              ? "OK" :
                code == RESULT_CANCELED        ? "Read canceled" :
                code == RESULT_NOT_FOUND       ? "Target not found" :
                code == RESULT_SERVER_ERROR    ? "Server reported error" :
                code == RESULT_TIMEOUT         ? "Read timed out" :
                code == RESULT_NO_READER       ? "No suitable ReaderWriter found" :
                code == RESULT_READER_ERROR    ? "ReaderWriter error" :
                code == RESULT_NOT_IMPLEMENTED ? "Not implemented" :
                code == RESULT_NOT_MODIFIED    ? "Not modified" :
                                                 "Unknown error";
        }

        std::string getResultCodeString() const
        {
            return getResultCodeString(ioCode);
        }
    };
}

ROCKY_SPECIALIZE_CONFIG(rocky::CachePolicy);
ROCKY_SPECIALIZE_CONFIG(rocky::ProxySettings);