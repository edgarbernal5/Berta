/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TEXT_EDITOR_HEADER
#define BT_TEXT_EDITOR_HEADER

#include <string>

namespace Berta
{
	class TextEditor
	{
	public:
		TextEditor() = default;

	private:
		std::wstring m_content;
		uint32_t m_caretPosition{ 0 };
	};
}

#endif