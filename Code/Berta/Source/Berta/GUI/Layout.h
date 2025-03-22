/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_LAYOUT_HEADER
#define BT_LAYOUT_HEADER

#include "Berta/Core/BasicTypes.h"

#include <memory>
#include <functional>
#include <map>

namespace Berta
{
    struct Window;
    class LayoutNode;
    class DockPaneLayoutNode;
    class DockPaneTabLayoutNode;
    class ControlBase;

    struct PaneInfo;

    struct Token
    {
        enum class Type
        {
            None,

            Identifier,
            NumberInt,
            NumberDouble,
            Percentage,
            String,
            OpenBrace,
            CloseBrace,
            Equal,
            Splitter,
            EndOfStream,

            VerticalLayout = 256,
            HorizontalLayout,
            Width,
            Height,

            MinHeight,
            MaxHeight,
            MinWidth,
            MaxWidth,

            Dock,
            DockPane
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
        "MaxWidth",
        "Dock",
        "DockPane"
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
        char m_identifier[g_maxIdentifierLength]{};
        int m_iValue{ 0 };
        double m_dValue{ 0.0 };

        Token::Type m_token{ Token::Type::EndOfStream };
    };

    class LayoutDockPaneEventsNotifier
    {
    public:
        virtual ~LayoutDockPaneEventsNotifier() = default;

        virtual void NotifyFloat(LayoutNode* node) = 0;
        virtual void RequestClose(LayoutNode* node) = 0;
    };

    class Layout : public LayoutDockPaneEventsNotifier
    {
    public:
        Layout();
        Layout(Window* window);
        ~Layout();

        //template<typename ...Args>
        //void AddPane(const std::string& paneId, Args & ... args);

        void AddPane(const std::string& paneId);
        void AddPaneTab(const std::string& paneId, const std::string& tabId, ControlBase* control);

        void Attach(const std::string& fieldId, Window* window);
        void Create(Window* window);
        void Parse(const std::string& source);

        void NotifyFloat(LayoutNode* node) override;
        void RequestClose(LayoutNode* node) override;
    private:
        class Parser
        {
        public:
            Parser(const std::string& source);

            std::unique_ptr<LayoutNode> Parse();

        private:
            bool Accept(Token::Type tokenId);
            bool AcceptIdentifier(std::string& identifier);
            bool Expect(Token::Type tokenId);
            bool IsEqualTo(Token::Type tokenId);

            Tokenizer m_tokenizer;
            std::string m_source;
        };

        DockPaneLayoutNode* GetPane(const std::string& paneId);
        DockPaneTabLayoutNode* GetPaneTab(const std::string& paneId, const std::string& tabId);

        Window* m_parent{ nullptr };
        std::unique_ptr<LayoutNode> m_rootNode;
        std::map<std::string, LayoutNode*> m_fields;
        std::map<std::string, DockPaneLayoutNode*> m_dockPaneFields;
        std::map<std::string, DockPaneTabLayoutNode*> m_dockPaneTabFields;
        std::vector<LayoutNode*> m_floatingDockFields;
        std::map<std::string, PaneInfo> m_dockPaneInfoFields;
    };

    //template<typename ...Args>
    //inline void Layout::AddPane(const std::string& paneId, Args & ...args)
    //{

    //}
}

#endif