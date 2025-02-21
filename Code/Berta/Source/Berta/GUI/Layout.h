/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_HEADER
#define BT_LAYOUT_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/LayoutNodes.h"
#include <memory>
#include <functional>
#include <map>

namespace Berta
{
    struct Window;

    struct Token
    {
        enum class Type
        {
            Identifier,
            NumberInt,
            NumberDouble,
            Percentage,
            String,
            OpenBrace,
            CloseBrace,
            Equal,
            Comma,
            Splitter,
            EndOfStream,

            VerticalLayout = 256,
            HorizontalLayout,
            Width,
            Height,

            MinHeight,
            MaxHeight,
            MinWidth,
            MaxWidth
        };

        Type type;
        std::string value;
        size_t line;
        size_t column;
    };

    static const int g_maxIdentifierLength = 255 + 1;
    static const char* g_reservedWords[] =
    {
        "VerticalLayout",
        "HorizontalLayout",
        "Width",
        "Height",

        "MinHeight",
        "MaxHeight",
        "MinWidth",
        "MaxWidth"
    };

    class Tokenizer
    {
    public:
        Tokenizer(const std::string& source);

        void Next();

        Token::Type GetToken() const
        {
            return m_token;
        }

        const char* GetIdentifier() const
        {
            return m_identifier;
        }

        int GetInt() const
        {
            return m_iValue;
        }

        double GetDouble() const
        {
            return m_dValue;
        }

        void GetTokenName(Token::Type token, char buffer[g_maxIdentifierLength]);
    private:
        bool SkipWhitespace();
        bool IsSymbol(char ch);
        bool IsNumberSeparator(char ch);
        bool ScanNumber();

        const char* m_buffer{ nullptr };
        const char* m_bufferEnd{ nullptr };
        int m_lineNumber{ 0 };
        bool m_error{ false };
        char m_identifier[g_maxIdentifierLength];
        int m_iValue{ 0 };
        double m_dValue{ 0.0 };

        Token::Type m_token{ Token::Type::EndOfStream };
    };

    class Layout
    {
    public:
        Layout();
        Layout(Window* window);
        ~Layout();

        void Attach(const std::string& fieldId, Window* window);
        void Create(Window* window);
        void Parse(const std::string& source);

    private:
        class Parser
        {
        public:
            Parser(const std::string& source);

            std::unique_ptr<LayoutNode> Parse();
            bool ParseAttributesOrNewBrace(std::unique_ptr<LayoutNode>&& newNode);


        private:
            bool Accept(Token::Type tokenId);
            bool AcceptIdentifier(std::string& identifier);
            bool Expect(Token::Type tokenId);
            bool IsEqualTo(Token::Type tokenId);

            Tokenizer m_tokenizer;
            std::string m_source;
        };

        std::unique_ptr<LayoutNode> m_rootNode;
        Window* m_parent{ nullptr };
        std::map<std::string, LayoutNode*> m_fields;
    };
}

#endif