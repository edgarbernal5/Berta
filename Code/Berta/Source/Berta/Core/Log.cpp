/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Log.h"

namespace Berta
{
	std::shared_ptr<Logger> Log::g_CoreLogger;

	void Log::Initialize()
	{
#if BT_DEBUG
		std::vector<std::shared_ptr<Sink>> coreSinks = {
			std::make_shared<ConsoleSink>()
		};

		/*std::vector<std::shared_ptr<Sink>> coreSinks = {
			std::make_shared<ConsoleSink>(),
			std::make_shared<FileSink>("LogTest.log"),
		};*/
#else
		/*std::vector<std::shared_ptr<Sink>> coreSinks = {
			std::make_shared<EmptySink>()
		};*/
		//std::vector<std::shared_ptr<Sink>> coreSinks;

		std::vector<std::shared_ptr<Sink>> coreSinks = {
			std::make_shared<ConsoleSink>()
		};
#endif

		g_CoreLogger = std::make_shared<Logger>(coreSinks.begin(), coreSinks.end());
	}

	void Log::Shutdown()
	{
		g_CoreLogger.reset();
	}
}