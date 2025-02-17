/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Layout.h"

#include "Berta/GUI/Window.h"

namespace Berta
{
	Layout::Layout() :
		m_parent(nullptr)
	{
	}

	Layout::Layout(Window* window)
	{
		Create(window);
	}

	Layout::~Layout()
	{
		m_fields.clear();
	}

	void Layout::Attach(const std::string& fieldId, Window* window)
	{
		auto& pair = m_fields[fieldId];

		if (pair == nullptr)
		{
			auto newNode = m_rootNode->Find(fieldId);
			pair = newNode;
		}
		pair->AddWindow(window);
	}

	void Layout::Create(Window* window)
	{
		if (!window)
		{
			return;
		}
		if (m_parent)
		{

		}
		m_parent = window;

		m_parent->Events->Resize.Connect([this](const ArgResize& args)
		{
			if (m_rootNode)
			{
				m_rootNode->SetArea({ 0, 0, args.NewSize.Width, args.NewSize.Height });
				m_rootNode->CalculateAreas();
				m_rootNode->Apply();
			}
		});

		m_parent->Events->Destroy.Connect([](const ArgDestroy& args)
		{
			//
		});
	}

	void Layout::Parse(const std::string& source)
	{
		Layout::Parser parser(source);

		bool result = parser.Parse(std::move(m_rootNode));
		if (!result)
		{
			return;
		}

		m_rootNode->SetParentWindow(m_parent);
		BT_CORE_TRACE << "Parse completed." << std::endl;
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
		if (m_buffer[0] == '=')
		{
			m_token = Token::Type::Equal;
			++m_buffer;
			return;
		}
		if (m_buffer[0] == '%')
		{
			m_token = Token::Type::Percentage;
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
		case '=':
		case '%':
			return true;
		}
		return false;
	}

	bool Tokenizer::IsNumberSeparator(char ch)
	{
		return ch == 0 || std::isspace(ch) || IsSymbol(ch);
	}

	bool Tokenizer::ScanNumber()
	{
		char* fEnd = nullptr;
		double dValue = std::strtod(m_buffer, &fEnd);

		if (fEnd == m_buffer)
		{
			return false;
		}

		char* iEnd = nullptr;
		long lValue = std::strtol(m_buffer, &iEnd, 10);

		if (fEnd > iEnd && IsNumberSeparator(fEnd[0]))
		{
			m_buffer = fEnd;
			m_token = Token::Type::NumberDouble; // it is a double.
			m_dValue = dValue;
			return true;
		}
		else if (iEnd > m_buffer && IsNumberSeparator(iEnd[0]))
		{
			m_buffer = iEnd;
			m_token = Token::Type::NumberInt; // it is a integer.
			m_iValue = static_cast<int>(lValue);
			return true;
		}
		return false;
	}

	Layout::Parser::Parser(const std::string& source) :
		m_source(source),
		m_tokenizer(source)
	{
	}

	bool Layout::Parser::Parse(std::unique_ptr<LayoutNode> && newNode)
	{
		if (Accept(Token::Type::OpenBrace))
		{
			bool isVertical = false;
			auto container = std::make_unique<LayoutNode>(isVertical);

			std::unique_ptr<LayoutNode> childNode;
			if (!ParseAttributesOrNewBrace(std::move(childNode)))
			{
				return false;
			}
			container->AddChild(std::move(childNode));
			newNode = std::move(container);
		}
		
		return true;
	}

	bool Layout::Parser::ParseAttributesOrNewBrace(std::unique_ptr<LayoutNode>&& newNode)
	{
		bool isVertical = false;
		std::string identifier;

		auto node = std::make_unique<LayoutNode>(isVertical);
		while (!Accept(Token::Type::CloseBrace))
		{
			if (Accept(Token::Type::EndOfStream))
				break;

			if (Accept(Token::Type::OpenBrace))
			{
				std::unique_ptr<LayoutNode> childNode;
				if (!ParseAttributesOrNewBrace(std::move(childNode)))
				{
					return false;
				}
				node->AddChild(std::move(childNode));
			}

			if (Accept(Token::Type::VerticalLayout))
			{
				isVertical = true;
				node->SetOrientation(isVertical);
			}
			else if (Accept(Token::Type::HorizontalLayout))
			{
				isVertical = false;
				node->SetOrientation(isVertical);
			}
			else if (AcceptIdentifier(identifier))
			{
				node->SetId(identifier);
			}
			else if (Accept(Token::Type::Width))
			{
				if (!Expect(Token::Type::Equal))
				{
					return false;
				}
				bool isNumberInt = false;
				bool isNumberDouble = false;
				if ((isNumberInt = Accept(Token::Type::NumberInt)) || (isNumberDouble = Accept(Token::Type::NumberDouble)))
				{
					Number number;
					if (isNumberInt)
					{
						number.scalar = m_tokenizer.GetInt();
					}
					else
					{
						number.scalar = m_tokenizer.GetDouble();
					}

					if (Accept(Token::Type::Percentage))
					{
						number.isPercentage = true;
					}
					node->SetProperty("Width", number);
				}
				else
				{
					return false;
				}
			}
			else if (Accept(Token::Type::Height))
			{
				if (!Expect(Token::Type::Equal))
				{
					return false;
				}
				bool isNumberInt = false;
				bool isNumberDouble = false;
				if ((isNumberInt = Accept(Token::Type::NumberInt)) || (isNumberDouble = Accept(Token::Type::NumberDouble)))
				{
					Number number;
					if (isNumberInt)
					{
						number.scalar = m_tokenizer.GetInt();
					}
					else
					{
						number.scalar = m_tokenizer.GetDouble();
					}

					if (Accept(Token::Type::Percentage))
					{
						number.isPercentage = true;
					}
					node->SetProperty("Height", number);
				}
				else
				{
					return false;
				}
			}
			else if (Accept(Token::Type::MinWidth))
			{
				if (!Expect(Token::Type::Equal))
				{
					return false;
				}
				bool isNumberInt = false;
				bool isNumberDouble = false;
				if ((isNumberInt = Accept(Token::Type::NumberInt)) || (isNumberDouble = Accept(Token::Type::NumberDouble)))
				{
					Number number;
					if (isNumberInt)
					{
						number.scalar = m_tokenizer.GetInt();
					}
					else
					{
						number.scalar = m_tokenizer.GetDouble();
					}

					if (Accept(Token::Type::Percentage))
					{
						number.isPercentage = true;
					}
					node->SetProperty("MinWidth", number);
				}
				else
				{
					return false;
				}
			}
			else if (Accept(Token::Type::MaxWidth))
			{
				if (!Expect(Token::Type::Equal))
				{
					return false;
				}
				bool isNumberInt = false;
				bool isNumberDouble = false;
				if ((isNumberInt = Accept(Token::Type::NumberInt)) || (isNumberDouble = Accept(Token::Type::NumberDouble)))
				{
					Number number;
					if (isNumberInt)
					{
						number.scalar = m_tokenizer.GetInt();
					}
					else
					{
						number.scalar = m_tokenizer.GetDouble();
					}

					if (Accept(Token::Type::Percentage))
					{
						number.isPercentage = true;
					}
					node->SetProperty("MaxWidth", number);
				}
				else
				{
					return false;
				}
				}
		}

		newNode = std::move(node);
		return true;
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

	void LayoutControlContainer::AddWindow(Window* window)
	{
		m_windows.push_back({ window });
	}
}