/**
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */
#pragma once

#include <rocky/Common.h>

#include <atomic>
#include <mutex>
#include <functional>
#include <vector>

namespace ROCKY_NAMESPACE
{
    /**
     * Easy way to add a thread-safe callback to a class.
     * 
     * Developer defines a callback, usually as a class member:
     *   Callback<void(int)> onClick;
     *
     * User adds a callback:
     *   instance->onClick([](int a) { ... });
     *
     * Class fires a callback:
     *   onClick(a);
     */
    template<typename F>
    class Callback
    {
    private:
        using Entry = typename std::pair<int, std::function<F>>;
        mutable int uidgen = 0;
        mutable std::vector<Entry> entries;
        mutable std::mutex mutex;
        mutable std::atomic_bool firing = { false };

    public:
        //! Adds a callback function
        int operator()(std::function<F>&& func) const {
            std::lock_guard<std::mutex> lock(mutex);
            auto uid = ++uidgen;
            entries.emplace_back(uid, func);
            return uid;
        }

        //! Removed a callback function with the UID returned from ()
        void remove(int uid) const {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
                if (iter->first == uid) {
                    entries.erase(iter);
                    break;
                }
            }
        }

        //! Executes all callback functions with the provided args
        template<typename... Args>
        void fire(Args&&... args) const {
            if (firing.exchange(true) == false) {
                std::lock_guard<std::mutex> lock(mutex);
                for (auto& e : entries)
                    e.second(args...);
                firing = false;
            }
        }
    };
}