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
        /*static Dispatcher& Get()
        {
            static Dispatcher instance;
            return instance;
        }*/
        Dispatcher();

        // Cualquier sistema puede encolar trabajo aquí
        void Enqueue(std::function<void()> action);

        // Se llama 1 sola vez por frame
        void ExecuteAll();

    private:
        std::vector<std::function<void()>> m_actions;
        std::mutex m_mutex;
    };
}

#endif
