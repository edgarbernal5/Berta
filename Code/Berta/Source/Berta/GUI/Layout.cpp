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
		Tokenizer tokenizer(text);
		auto tokens = tokenizer.Tokenize();

		Layout::Parser parser(tokens);
		auto rootLayout = parser.Parse();

		m_rootNode = std::move(rootLayout);
	}

	Tokenizer::Tokenizer(const std::string& source) : 
		m_source(source)
	{

	}

	std::vector<Token> Tokenizer::Tokenize()
	{
		std::vector<Token> tokens;

		if (m_source.empty())
			return tokens;

		size_t line = 1, column = 1;

		for (size_t i = 0; i < m_source.size();)
		{
			char ch = m_source[i];

			if (std::isspace(ch))
			{
				if (ch == '\n')
				{
					++line;
					column = 1;
				}
				else
				{
					++column;
				}
				++i;
			}
			else if (std::isalpha(ch) || ch == '_')
			{
				// Parse identifiers (e.g., "VerticalLayout")
				size_t start = i;
				while (std::isalnum(m_source[i]) || m_source[i] == '_')
					++i;
				
				tokens.push_back({ Token::Type::Identifier, m_source.substr(start, i - start), line, column });
				column += (i - start);
			}
			else if (std::isdigit(ch) || (ch == '-' && i + 1 < m_source.size() && std::isdigit(m_source[i + 1])))
			{
				// Parse numbers (e.g., "42" or "-3.14")
				size_t start = i;
				while (std::isdigit(m_source[i]) || m_source[i] == '.')
					++i;

				tokens.push_back({ Token::Type::Number, m_source.substr(start, i - start), line, column });
				column += (i - start);
			}
			else if (ch == '{')
			{
				tokens.push_back({ Token::Type::OpenBrace, "{", line, column++ });
				++i;
			}
			else if (ch == '}')
			{
				tokens.push_back({ Token::Type::CloseBrace, "}", line, column++ });
				++i;
			}
			else if (ch == ':')
			{
				tokens.push_back({ Token::Type::Colon, ":", line, column++ });
				++i;
			}
			else if (ch == ',')
			{
				tokens.push_back({ Token::Type::Comma, ",", line, column++ });
				++i;
			}
			else if (ch == '"')
			{
				// Parse strings (e.g., `"Hello"`)
				size_t start = ++i;
				while (i < m_source.size() && m_source[i] != '"') 
					++i;
				
				tokens.push_back({ Token::Type::String, m_source.substr(start, i - start), line, column });
				column += (i - start + 2); // Include quotes
				++i;
			}
			else
			{
				throw std::runtime_error("Unexpected character in layout file.");
			}

		}

		tokens.push_back({ Token::Type::EndOfFile, "", line, column });
		return tokens;
	}

	Layout::Parser::Parser(const std::vector<Token>& tokens) :
		m_tokens(tokens)
	{
	}

	std::unique_ptr<LayoutNode> Layout::Parser::Parse()
	{
		return std::unique_ptr<LayoutNode>();
	}
}