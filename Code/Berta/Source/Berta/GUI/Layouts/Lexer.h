/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LEXER_HEADER
#define BT_LEXER_HEADER

#include "Berta/GUI/Layouts/BasicTypes.h"

namespace Berta
{
    class Lexer
    {
    public:
        explicit Lexer(std::string_view source);
        
        [[nodiscard]] std::vector<Token> Tokenize();

    private:
        char Peek() const { return IsAtEnd() ? '\0' : m_source[m_position]; }
        char Advance() { return m_source[m_position++]; }
        bool IsAtEnd() const { return m_position >= m_source.length(); }
        
        // Helper para extraer el string_view sin copiar
        std::string_view CurrentView(size_t length) const
        {
            return m_source.substr(m_position - length, length);
        }

        void SkipWhitespace();
        Token ReadIdentifierOrKeyword();
        Token ReadNumber();
        Token ReadString();
        
        std::string_view m_source;
        size_t m_position;
    };
}

#endif
