#include "btpch.h"
#include "Lexer.h"

namespace Berta
{
    Lexer::Lexer(std::string_view source) : 
        m_source(source), m_position(0)
    {
    }

    std::vector<Token> Lexer::Tokenize()
    {
        std::vector<Token> tokens;
        tokens.reserve(128); 

        while (!IsAtEnd())
        {
            SkipWhitespace();
            
            if (IsAtEnd())
            {
                break;
            }

            char ch = Peek();

            if (std::isalpha(ch) || ch == '_')
            {
                tokens.push_back(ReadIdentifierOrKeyword());
            } 
            else if (ch == '"')
            {
                tokens.push_back(ReadString());
            }
            else if (std::isdigit(ch) || ch == '-')
            {
                tokens.push_back(ReadNumber());
            }
            else
            {
                // Símbolos de 1 carácter
                Advance();
                
                switch (ch)
                {
                case '=': tokens.push_back({Token::Type::Equal, m_source.substr(m_position - 1, 1)}); break;
                case ':': tokens.push_back({Token::Type::Colon, m_source.substr(m_position - 1, 1)}); break;
                case '{': tokens.push_back({Token::Type::OpenBrace, m_source.substr(m_position - 1, 1)}); break;
                case '}': tokens.push_back({Token::Type::CloseBrace, m_source.substr(m_position - 1, 1)}); break;
                case '|': tokens.push_back({Token::Type::Splitter, m_source.substr(m_position - 1, 1)}); break;
                case '%': tokens.push_back({Token::Type::Percentage, m_source.substr(m_position - 1, 1)}); break;
                default:  tokens.push_back({Token::Type::Unknown, m_source.substr(m_position - 1, 1)}); break;
                }
            }
        }

        tokens.push_back({Token::Type::EndOfStream, ""});
        return tokens;
    }

    void Lexer::SkipWhitespace()
    {
        while (!IsAtEnd() && std::isspace(Peek()))
        {
            Advance();
        }
    }

    Token Lexer::ReadIdentifierOrKeyword()
    {
        size_t start = m_position;
        while (!IsAtEnd() && (std::isalnum(Peek()) || Peek() == '_'))
        {
            Advance();
        }

        std::string_view text = m_source.substr(start, m_position - start);

        // Mapeo directo a palabras clave para no hacer esto en el Parser
        Token::Type type = Token::Type::Identifier;
        if (text == "VerticalLayout") type = Token::Type::VerticalLayout;
        else if (text == "HorizontalLayout") type = Token::Type::HorizontalLayout;
        else if (text == "Dock") type = Token::Type::Dock;
        else if (text == "DockPane") type = Token::Type::DockPane;
        else if (text == "Width") type = Token::Type::Width;
        else if (text == "Height") type = Token::Type::Height;
        else if (text == "MinHeight") type = Token::Type::MinHeight;
        else if (text == "MinWidth") type = Token::Type::MinWidth;
        else if (text == "MaxHeight") type = Token::Type::MaxHeight;
        else if (text == "MaxWidth") type = Token::Type::MaxWidth;

        return { type, text };
    }

    Token Lexer::ReadNumber()
    {
        size_t start = m_position;
        Advance(); // Consumir el primer dígito o el '-'
        while (!IsAtEnd() && (std::isdigit(Peek()) || Peek() == '.'))
        {
            Advance();
        }

        return { Token::Type::NumberInt, m_source.substr(start, m_position - start) };
    }

    Token Lexer::ReadString()
    {
        Advance(); // Consumir la comilla inicial '"'
        size_t start = m_position;
        while (!IsAtEnd() && Peek() != '"')
        {
            Advance();
        }
        
        std::string_view text = m_source.substr(start, m_position - start);
        if (!IsAtEnd())
        {
            Advance(); // Consumir la comilla final '"'
        }
        return { Token::Type::String, text };
    }
}
