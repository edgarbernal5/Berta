/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Logger.h"

namespace Berta
{
	ConsoleSink::ConsoleSink() :
		m_stream{ std::cout }
	{
	}

	ConsoleSink::ConsoleSink(std::ostream& stream) :
		m_stream{ stream }
	{
	}

	void ConsoleSink::Commit(const std::string& outputMessage)
	{
		m_stream << outputMessage;
	}

	Logger::Logger(const Logger& other) :
		m_sinks(other.m_sinks),
		m_logLevel(other.m_logLevel)
	{
	}

	FileSink::FileSink(const std::string& filename)
	{
		m_stream.open(filename);
	}

	FileSink::~FileSink()
	{
		m_stream.close();
	}

	void FileSink::Commit(const std::string& outputMessage)
	{
		m_stream << outputMessage;
		//m_stream.flush();
	}

	void EmptySink::Commit(const std::string& outputMessage)
	{
	}
}