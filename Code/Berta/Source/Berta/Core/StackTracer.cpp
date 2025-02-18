/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "StackTracer.h"

#if BT_PLATFORM_WINDOWS
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")
#endif

//#include <stacktrace> TODO: C++23

namespace Berta
{
	StackTracer::StackTracer()
	{
#if BT_PLATFORM_WINDOWS
        m_process = ::GetCurrentProcess();
        ::SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS); // Optimize symbol loading
        ::SymInitialize(m_process, nullptr, TRUE);
#endif
	}

	StackTracer::~StackTracer()
	{
#if BT_PLATFORM_WINDOWS
        ::SymCleanup(m_process);
#endif
	}

	std::string StackTracer::GetStackTrace(int offset, int maxFrames) const
	{
#if BT_PLATFORM_WINDOWS
        std::stringstream ss;
        void* callStack[128];
        unsigned short frames = CaptureStackBackTrace(0, 128, callStack, nullptr);

        SYMBOL_INFO* symbol = (SYMBOL_INFO*)::calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        int endFrame = (std::min)(static_cast<int>(frames), maxFrames);

        for (int i = offset; i < offset + endFrame; ++i)
        {
            DWORD64 address = (DWORD64)(callStack[i]);

            if (::SymFromAddr(m_process, address, 0, symbol))
            {
                ss << frames - i - 1 << ": " << '\t' << symbol->Name << " - 0x"
                    << std::hex << symbol->Address << std::dec << "\n";
            }
            else
            {
                ss << frames - i - 1 << ": " << '\t' << "[Unknown] - 0x"
                    << std::hex << address << std::dec << "\n";
            }
        }

        ss << "\n";
        free(symbol);

        return ss.str();
#else
        return "";
#endif
	}
}