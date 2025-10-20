/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "DrawBatchActivator.h"

#include "Berta/GUI/Window.h"

namespace Berta
{
    std::unordered_map< Window*, DrawBatcherContext> DrawBatchActivator::g_contexts;
    DrawBatchActivator::DrawBatchActivator(Window* rootWindow) : 
        m_context(g_contexts[rootWindow->RootWindow])
    {
        /*if (m_context.m_rootWindow)
            return;

        m_context.m_rootWindow = rootWindow->RootWindow;
        rootWindow->RootWindow->DrawBatch = this;*/
    }

    DrawBatchActivator::~DrawBatchActivator()
    {
        //if (!m_context.m_rootWindow || m_context.m_rootWindow->Flags.IsDisposed)
        //    return;

        //if (m_context.m_rootWindow->DeferredCounter > 0)
        //    return;

        ////Call RefreshWindow
        //std::cout << " DrawBatchActivator destructor. wnd=" << m_context.m_rootWindow->Name << ". queue=" << m_context.m_rootWindow->Flags.isQueuingBatch << std::endl;
        //if (m_context.m_rootWindow->Flags.isQueuingBatch)
        //{
        //    m_context.m_rootWindow->Flags.isQueuingBatch = false;
        //    API::RefreshWindow(m_context.m_rootWindow->RootHandle);
        //}
        //m_context.m_rootWindow->DrawBatch = nullptr;
        //m_context.m_rootWindow = nullptr;
    }
}