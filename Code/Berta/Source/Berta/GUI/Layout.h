/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_HEADER
#define BT_LAYOUT_HEADER

#include "Berta/Core/BasicTypes.h"

namespace Berta
{
    struct Token
    {
        enum class Type
        {
            Identifier,
            Number,
            String,
            OpenBrace,   // '{'
            CloseBrace,  // '}'
            Colon,       // ':'
            Comma,       // ','
            EndOfFile
        };

        Type type;
        std::string value;
        size_t line;
        size_t column;
    };

    class LayoutNode;

    class Layout
    {
    public:

        void Parse(const std::string& text);

    private:
        class Parser
        {
        public:
            Parser(const std::vector<Token>& tokens);

            std::unique_ptr<LayoutNode> Parse();
            std::vector<Token> m_tokens;
        };
        std::unique_ptr<LayoutNode> m_rootNode;
    };

    class Tokenizer
    {
    public:
        Tokenizer(const std::string& source);

        std::vector<Token> Tokenize();

    private:
        std::string m_source;
    };
}

#endif