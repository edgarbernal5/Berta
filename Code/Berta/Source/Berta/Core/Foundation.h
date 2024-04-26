/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FOUNDATION_HEADER
#define BT_FOUNDATION_HEADER

namespace Berta
{
	class Logger;

	class Foundation
	{
	public:
		Foundation();
		~Foundation();

		Foundation(const Foundation&) = delete;
		Foundation& operator=(const Foundation&) = delete;

		static Foundation& Instance();
	private:
		static Foundation g_foundation;
		std::shared_ptr<Logger> m_logger;
	};
}

#endif