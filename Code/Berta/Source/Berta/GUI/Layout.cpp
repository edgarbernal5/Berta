/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Layout.h"

#include "Berta/GUI/LayoutNodes.h"

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

	Tokenizer::Tokenizer(const std::string& source) : m_source(source)
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
			++i;
		}

		tokens.push_back({ Token::Type::EndOfFile, "", line, column });
		return tokens;
	}

	Layout::Parser::Parser(const std::vector<Token>& tokens) : m_tokens(tokens)
	{
	}

	std::unique_ptr<LayoutNode> Layout::Parser::Parse()
	{
		return std::unique_ptr<LayoutNode>();
	}
}