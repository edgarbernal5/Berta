/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_EVENT_HEADER
#define BT_EVENT_HEADER

#include <functional>
#include <memory>
#include <vector>
#include <algorithm>
#include <mutex>

namespace Berta
{
    using EventHandlerId = size_t;

    template <typename Argument>
    class Event
    {
    public:
        using Handler = std::function<void(const Argument&)>;

    public:
        Event() : m_data(std::make_shared<Data>()) {}
        Event(Event&& other) : Event() { *this = std::move(other); }
        Event(const Event&) = default;

        Event& operator=(const Event&) = default;
        Event& operator=(Event&& other)
        {
            std::swap(m_data, other.m_data);
            return *this;
        }

    private:
        struct StoredHandler
        {
            EventHandlerId Id;
            std::shared_ptr<Handler> Callback;
            struct
            {
                bool Once : 1;
                bool Triggered : 1;
            }Flags;
        };

        using HandlerList = std::vector<StoredHandler>;

        struct Data
        {
            EventHandlerId IdCounter = 0;
            HandlerList Observers;
            std::mutex ObserverMutex;
        };

        std::shared_ptr<Data> m_data;

        EventHandlerId AddHandler(Handler handler, bool once = false) const
        {
            std::lock_guard<std::mutex> lock(m_data->ObserverMutex);
            m_data->Observers.emplace_back(StoredHandler{ m_data->IdCounter, std::make_shared<Handler>(handler), {once, false} });
            return m_data->IdCounter++;
        }

    public:
        EventHandlerId Connect(const Handler& handler) const
        {
            return AddHandler(handler);
        }

        EventHandlerId ConnectOnce(const Handler& handler) const
        {
            return AddHandler(handler, true);
        }

        void Disconnect(EventHandlerId id) const
        {
            std::lock_guard<std::mutex> lock(m_data->ObserverMutex);
            auto it = std::find_if(m_data->Observers.begin(), m_data->Observers.end(),
                [&](auto& observer)
                {
                    return observer.Id == id;
                });

            if (it != m_data->Observers.end())
            {
                m_data->Observers.erase(it);
            }
        }

        size_t Length() const
        {
            std::lock_guard<std::mutex> lock(m_data->ObserverMutex);
            return m_data->Observers.size();
        }

        void Reset() const
        {
            std::lock_guard<std::mutex> lock(m_data->ObserverMutex);
            m_data->Observers.clear();
        }

        void Emit(Argument& args) const
        {
            std::vector<std::weak_ptr<Handler>> handlers;
            {
                std::lock_guard<std::mutex> lock(m_data->ObserverMutex);
                handlers.resize(m_data->Observers.size());
                std::transform(m_data->Observers.begin(), m_data->Observers.end(), handlers.begin(),
                    [](auto& h) { return h.Callback; });
            }

            for (auto& weakCallback : handlers)
            {
                if (auto callback = weakCallback.lock())
                {
                    (*callback)(args);
                }
            }
        }
    };
}

#endif