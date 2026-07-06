/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_UI_DISPATCHER_HEADER
#define BT_UI_DISPATCHER_HEADER

#include <functional>
#include <vector>
#include <mutex>

namespace Berta
{
    class Dispatcher
    {
    public:
        Dispatcher();

        // Cualquier sistema puede encolar trabajo aquí
        void Enqueue(std::function<void()> action);

        // Se llama 1 sola vez por frame
        void ExecuteAll();

        static Dispatcher& Get()
        {
            static Dispatcher instance;
            return instance;
        }
    private:
        std::vector<std::function<void()>> m_actions;
        std::mutex m_mutex;
    };
}

#endif
