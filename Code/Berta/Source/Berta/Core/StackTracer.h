/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_STACK_TRACER_HEADER
#define BT_STACK_TRACER_HEADER

#include <string>

namespace Berta
{
    class StackTracer
    {
    public:
        StackTracer();
        ~StackTracer();

        std::string GetStackTrace(int offset = 3, int maxFrames = 3) const;
    
    private:
#if BT_PLATFORM_WINDOWS
        HANDLE m_process;
#endif
    };
}

#endif