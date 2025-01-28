/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Layout.h"

namespace Berta
{
	void Layout::Parse(const std::string& text)
	{
		Layout::Parser parser(text);
		auto rootLayout = parser.Parse();

		m_rootNode = std::move(rootLayout);
	}

	Tokenizer::Tokenizer(const std::string& source) : 
		m_buffer(source.c_str()),
		m_bufferEnd(source.c_str() + source.size())
	{
		Next();
	}

	void Tokenizer::Next()
	{
		while (SkipWhitespace()) {}

		if (m_error)
		{
			m_token = Token::Type::EndOfStream;
			return;
		}

		if (m_buffer >= m_bufferEnd || *m_buffer == '\0')
		{
			m_token = Token::Type::EndOfStream;
			return;
		}

		if (m_buffer[0] == '{')
		{
			m_token = Token::Type::OpenBrace;
			++m_buffer;
			return;
		}
		if (m_buffer[0] == '}')
		{
			m_token = Token::Type::CloseBrace;
			++m_buffer;
			return;
		}

		const char* start = m_buffer;
		while (m_buffer < m_bufferEnd && m_buffer[0] != 0 && !IsSymbol(m_buffer[0]) && !std::isspace(m_buffer[0]))
		{
			++m_buffer;
		}

		size_t length = m_buffer - start;
		memcpy(m_identifier, start, length);
		m_identifier[length] = 0;

		const int numReservedWords = sizeof(g_reservedWords) / sizeof(const char*);
		for (int i = 0; i < numReservedWords; ++i)
		{
			if (strcmp(g_reservedWords[i], m_identifier) == 0)
			{
				m_token = (Token::Type)(i + 256);
				return;
			}
		}

		m_token = Token::Type::Identifier;

		//size_t line = 1, column = 1;

		//for (size_t i = 0; i < m_source.size();)
		//{
		//	char ch = m_source[i];

		//	if (std::isspace(ch))
		//	{
		//		if (ch == '\n')
		//		{
		//			++line;
		//			column = 1;
		//		}
		//		else
		//		{
		//			++column;
		//		}
		//		++i;
		//	}
		//	else if (std::isalpha(ch) || ch == '_')
		//	{
		//		// Parse identifiers (e.g., "VerticalLayout")
		//		size_t start = i;
		//		while (std::isalnum(m_source[i]) || m_source[i] == '_')
		//			++i;
		//		
		//		tokens.push_back({ Token::Type::Identifier, m_source.substr(start, i - start), line, column });
		//		column += (i - start);
		//	}
		//	else if (std::isdigit(ch) || (ch == '-' && i + 1 < m_source.size() && std::isdigit(m_source[i + 1])))
		//	{
		//		// Parse numbers (e.g., "42" or "-3.14")
		//		size_t start = i;
		//		while (std::isdigit(m_source[i]) || m_source[i] == '.')
		//			++i;

		//		tokens.push_back({ Token::Type::Number, m_source.substr(start, i - start), line, column });
		//		column += (i - start);
		//	}
		//	else if (ch == '{')
		//	{
		//		tokens.push_back({ Token::Type::OpenBrace, "{", line, column++ });
		//		++i;
		//	}
		//	else if (ch == '}')
		//	{
		//		tokens.push_back({ Token::Type::CloseBrace, "}", line, column++ });
		//		++i;
		//	}
		//	else if (ch == ':')
		//	{
		//		tokens.push_back({ Token::Type::Colon, ":", line, column++ });
		//		++i;
		//	}
		//	else if (ch == ',')
		//	{
		//		tokens.push_back({ Token::Type::Comma, ",", line, column++ });
		//		++i;
		//	}
		//	else if (ch == '"')
		//	{
		//		// Parse strings (e.g., `"Hello"`)
		//		size_t start = ++i;
		//		while (i < m_source.size() && m_source[i] != '"') 
		//			++i;
		//		
		//		tokens.push_back({ Token::Type::String, m_source.substr(start, i - start), line, column });
		//		column += (i - start + 2); // Include quotes
		//		++i;
		//	}
		//	else
		//	{
		//		throw std::runtime_error("Unexpected character in layout file.");
		//	}

		//}

		//tokens.push_back({ Token::Type::EndOfStream, "", line, column });
		//return tokens;
	}

	void Tokenizer::GetTokenName(Token::Type token, char buffer[g_maxIdentifierLength])
	{
	}

	bool Tokenizer::SkipWhitespace()
	{
		bool result = false;
		while (m_buffer < m_bufferEnd && std::isspace(m_buffer[0]))
		{
			result = true;
			if (m_buffer[0] == '\n')
			{
				++m_lineNumber;
			}
			++m_buffer;
		}
		return result;
	}

	bool Tokenizer::IsSymbol(char ch)
	{
		switch (ch)
		{
		case '{':
		case '}':
		case '-':
			return true;
		}
		return false;
	}

	Layout::Parser::Parser(const std::string& text) :
		m_text(text),
		m_tokenizer(text)
	{
	}

	std::unique_ptr<LayoutNode> Layout::Parser::Parse()
	{
		/*if (m_tokens.empty() || m_currentTokenIndex >= m_tokens.size())
		{
			return nullptr;
		}*/

		/*
		{ VerticalLayout a }
		{ HorizontalLayout a }

		{ { ee height =30% } {} }
		{ VerticalLayout { aa } {bb} }

		{ VerticalLayout {menuBar height=25} {dock}}
		*/
		std::unique_ptr<LayoutNode> node;

		while (!Accept(Token::Type::EndOfStream))
		{
			if (Accept(Token::Type::OpenBrace))
			{
				bool isVertical = false;
				auto container = std::make_unique<ContainerLayout>(isVertical);

				auto child = Parse();
				container->AddChild(std::move(child));

			}
		}
		//bool isRunning = true;
		//Token& token = GetNext();
		//while (isRunning && token.type != Token::Type::EndOfStream)
		//{
		//	switch (token.type)
		//	{
		//	case Token::Type::OpenBrace:
		//	{
		//		bool isVertical = false;
		//		auto container = std::make_unique<ContainerLayout>(isVertical);

		//		auto child = Parse();
		//		container->AddChild(std::move(child));

		//		/*if (token.type == Token::Type::CloseBrace)
		//		{
		//			node = std::move(container);
		//			token = GetNext();
		//		}
		//		else
		//		{
		//			BT_CORE_ERROR << "error. expected close brace }" << std::endl;
		//		}*/

		//		break;
		//	}
		//	case Token::Type::CloseBrace:
		//	{
		//		bool isVertical = false;
		//		auto container = std::make_unique<ContainerLayout>(isVertical);

		//		node = std::move(container);
		//		token = GetNext();
		//		isRunning = false;
		//		break;
		//	}
		//	case Token::Type::Identifier:
		//	{
		//		if (token.value == "VerticalLayout" || token.value == "HorizontalLayout")
		//		{
		//			bool isVertical = token.value == "VerticalLayout";
		//			auto container = std::make_unique<ContainerLayout>(isVertical);

		//			Token innerToken = GetNext();
		//			if (innerToken.type == Token::Type::Identifier)
		//			{
		//				container->SetId(innerToken.value);
		//			}
		//			else
		//			{
		//			}

		//			node = std::move(container);
		//		}
		//		break;
		//	}
		//	default:
		//		break;
		//	}
		//	//token = GetNext();
		//}
		return node;
	}

	Token Layout::Parser::GetNext()
	{
		/*if (m_currentTokenIndex < m_tokens.size())
		{
			return m_tokens[m_currentTokenIndex++];
		}*/
		return { Token::Type::EndOfStream };
	}

	bool Layout::Parser::Accept(Token::Type tokenId)
	{
		if (m_tokenizer.GetToken() == tokenId)
		{
			m_tokenizer.Next();
			return true;
		}
		return false;
	}

	bool Layout::Parser::Expect(Token::Type tokenId)
	{
		if (!Accept(tokenId))
		{
			/*
			char want[HLSLTokenizer::s_maxIdentifier];
			m_tokenizer.GetTokenName(token, want);
			*/
			BT_CORE_ERROR << "error. expected token= " << (int)tokenId << std::endl;
			return false;
		}
		return true;
	}
}