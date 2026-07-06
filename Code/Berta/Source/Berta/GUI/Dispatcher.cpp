/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Dispatcher.h"

namespace Berta
{
    Dispatcher::Dispatcher()
    {
        m_actions.reserve(10);
    }

    void Dispatcher::Enqueue(std::function<void()> action)
    {
        // Mutex es opcional si garantizas que todo pasa en el Main Thread,
        // pero es buena práctica prepararse para Multithreading.
        std::lock_guard<std::mutex> lock(m_mutex); 
        m_actions.push_back(std::move(action));
    }

    void Dispatcher::ExecuteAll()
    {
        std::vector<std::function<void()>> tempActions;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            tempActions = std::move(m_actions);
            // m_actions ahora está vacío y listo para el siguiente frame
        }

        for (auto& action : tempActions)
        {
            action();
        }
    }
}
