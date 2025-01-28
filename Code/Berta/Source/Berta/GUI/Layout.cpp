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

		bool result = parser.Parse(std::move(m_rootNode));
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

		if (ScanNumber())
		{
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

	bool Tokenizer::ScanNumber()
	{
		char* fEnd = nullptr;
		double dValue = std::strtod(m_buffer, &fEnd);

		if (fEnd == m_buffer)
		{
			return false;
		}
		if (fEnd < m_bufferEnd && fEnd[0] == '%')
		{
			// no sabemos si es un entero o double.
			m_token = Token::Type::Number; // it is a double.
			return true;
		}

		char* iEnd = nullptr;
		long lValue = std::strtol(m_buffer, &iEnd, 10);

		if (fEnd > iEnd && (fEnd[0] == 0))
		{
			m_buffer = fEnd;
			m_token = Token::Type::Number; // it is a double.
			return true;
		}
		else if (iEnd > m_buffer && (iEnd[0] == 0))
		{
			m_buffer = iEnd;
			m_token = Token::Type::Number; // it is a integer.
			return true;
		}
		if (iEnd < m_bufferEnd && iEnd[0] == '%')
		{
			return true;
		}
		return false;
	}

	Layout::Parser::Parser(const std::string& text) :
		m_text(text),
		m_tokenizer(text)
	{
	}

	bool Layout::Parser::Parse(std::unique_ptr<LayoutNode> && newNode)
	{
		/*
		{ VerticalLayout a }
		{ HorizontalLayout a }

		{ { ee height =30% } {} }
		{ VerticalLayout { aa } {bb} }

		{ VerticalLayout {menuBar height=25} {dock}}
		*/

		while (!Accept(Token::Type::EndOfStream))
		{
			if (Accept(Token::Type::OpenBrace))
			{
				bool isVertical = false;
				auto container = std::make_unique<ContainerLayout>(isVertical);

				std::unique_ptr<LayoutNode> childNode;
				if (!ParseAttributesOrNewBrace(std::move(childNode)))
				{
					return false;
				}
				container->AddChild(std::move(childNode));
				newNode = std::move(container);
			}
		}
		return true;
	}

	bool Layout::Parser::ParseAttributesOrNewBrace(std::unique_ptr<LayoutNode>&& newNode)
	{
		
		bool isVertical = false;
		std::string identifier;

		auto node = std::make_unique<ContainerLayout>(isVertical);
		while (!Accept(Token::Type::CloseBrace))
		{
			if (Accept(Token::Type::OpenBrace))
			{
				std::unique_ptr<LayoutNode> childNode;
				if (!ParseAttributesOrNewBrace(std::move(childNode)))
				{
					return false;
				}
				node->AddChild(std::move(childNode));
			}

			if (Accept(Token::Type::VerticalLayout) || Accept(Token::Type::HorizontalLayout))
			{
				isVertical = true;
				node->SetOrientation(isVertical);
			}
			else if (AcceptIdentifier(identifier))
			{
				node->SetId(identifier);
			}
			else if (Accept(Token::Type::Width))
			{

			}
			else if (Accept(Token::Type::Height))
			{

			}
		}

		newNode = std::move(node);
		return true;
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

	bool Layout::Parser::AcceptIdentifier(std::string& identifier)
	{
		if (m_tokenizer.GetToken() == Token::Type::Identifier)
		{
			identifier = m_tokenizer.GetIdentifier();
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