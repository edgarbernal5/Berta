/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_PARSER_HEADER
#define BT_LAYOUT_PARSER_HEADER

#include <map>
#include <string_view>
#include <memory>
#include <stdexcept>
#include <variant>

#include "Berta/GUI/Layouts/BasicTypes.h"

namespace Berta
{
    class LayoutNode;
    class ContainerLayoutNode;
    
    class LayoutParser
    {
    public:
        explicit LayoutParser(const std::vector<Token>& tokens);
        ~LayoutParser() = default;

        [[nodiscard]] std::unique_ptr<LayoutNode> Parse();

    private:
        bool Accept(Token::Type expectedType);
        bool Expect(Token::Type expectedType);
        bool AcceptIdentifier(std::string_view& outIdentifier);
        
        void ParseProperty(std::map<std::string, PropertyValue, std::less<>>& outProperties);
        
        std::unique_ptr<LayoutNode> ParseNode();
        
        [[nodiscard]] bool IsAtEnd() const;
        [[nodiscard]] const Token& Peek() const;
        const Token& Advance();
        
        const std::vector<Token>& m_tokens;
        size_t m_currentIndex{ 0 };
        static inline const Token g_endTokenType{ Token::Type::EndOfStream, "" };
    };
}


#endif
