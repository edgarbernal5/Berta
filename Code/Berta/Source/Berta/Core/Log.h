/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LOG_HEADER
#define BT_LOG_HEADER

#include "Logger.h"

#define BT_CORE_TRACE (*Berta::Log::GetCoreLogger())(LogLevel::Trace)
#define BT_CORE_DEBUG (*Berta::Log::GetCoreLogger())(LogLevel::Debug)
#define BT_CORE_WARN (*Berta::Log::GetCoreLogger())(LogLevel::Warn)
#define BT_CORE_ERROR (*Berta::Log::GetCoreLogger())(LogLevel::Error)

namespace Berta
{
	class Log
	{
	public:
		static void Initialize();
		static void Shutdown();

		static std::shared_ptr<Logger>& GetCoreLogger() { return g_CoreLogger; }

		template<typename... Args>
		static void PrintAssertMessage(const char* file, int line, const char* message, Args&&... args);
	private:
		static std::shared_ptr<Logger> g_CoreLogger;
	};

	template<typename... Args>
	void Log::PrintAssertMessage(const char* file, int line, const char* message, Args&&... args)
	{
		//https://stackoverflow.com/questions/27375089/what-is-the-easiest-way-to-print-a-variadic-parameter-pack-using-stdostream
		//https://learn.microsoft.com/en-us/cpp/cpp/ellipses-and-variadic-templates?view=msvc-170

		std::ostringstream builder;
		builder << message;
		
		((builder << ". " << std::forward<Args>(args)), ...);
		builder << std::endl << "File: " << file << ". Line: " << line;
		
		(*g_CoreLogger)(LogLevel::Error) << builder.str() << std::endl;
	}
}

#endif