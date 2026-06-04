#include "btpch.h"
#include "LayoutParser.h"

#include <charconv>

#include "LayoutNodes.h"

namespace Berta
{
    LayoutParser::LayoutParser(const std::vector<Token>& tokens)
        : m_tokens(tokens)
    {
    }

    std::unique_ptr<LayoutNode> LayoutParser::Parse()
    {
        auto rootNode = ParseNode();
    
        if (!IsAtEnd() && Peek().type != Token::Type::EndOfStream)
        {
            throw std::runtime_error("Error de sintaxis: Datos basura al final del archivo de layout.");
        }
    
        return rootNode;
    }

    bool LayoutParser::Accept(Token::Type expectedType)
    {
        if (IsAtEnd())
        {
            return false;
        }
        
        if (Peek().type == expectedType)
        {
            Advance();
            return true;
        }
        return false;
    }

    bool LayoutParser::Expect(Token::Type expectedType)
    {
        if (Accept(expectedType))
        {
            return true;
        }

        throw std::runtime_error("Error de sintaxis en el Layout: Token inesperado.");
    }

    bool LayoutParser::AcceptIdentifier(std::string_view& outIdentifier)
    {
        if (!IsAtEnd() && Peek().type == Token::Type::Identifier)
        {
            outIdentifier = Peek().value;
            Advance();
            return true;
        }
        return false;
    }

    void LayoutParser::ParseProperty(std::map<std::string, PropertyValue, std::less<>>& outProperties)
    {
        Token propToken = Advance(); 
    
        std::string propertyName;
        if (propToken.type == Token::Type::Width) propertyName = "Width";
        else if (propToken.type == Token::Type::Height) propertyName = "Height";
        else if (propToken.type == Token::Type::MinHeight) propertyName = "MinHeight";
        else if (propToken.type == Token::Type::MaxHeight) propertyName = "MaxHeight";
        else if (propToken.type == Token::Type::MinWidth) propertyName = "MinWidth";
        else if (propToken.type == Token::Type::MaxWidth) propertyName = "MaxWidth";

        Expect(Token::Type::Equal);

        Token valueToken = Peek();
        double rawValue = 0.0;
        
        if (Accept(Token::Type::NumberInt) || Accept(Token::Type::NumberDouble))
        {
            // C++17 std::from_chars: Conversión de alto rendimiento basada en string_view sin alocaciones
            auto [ptr, ec] = std::from_chars(valueToken.value.data(), valueToken.value.data() + valueToken.value.size(), rawValue);
        
            if (ec != std::errc{})
            {
                throw std::runtime_error("Error al parsear el valor numérico de la propiedad.");
            }
        }
        else 
        {
            throw std::runtime_error("Se esperaba un número tras el '=' en el layout.");
        }

        Berta::DimensionUnit unit = Berta::DimensionUnit::Pixels;
        if (Accept(Token::Type::Percentage))
        {
            unit = Berta::DimensionUnit::Percentage;
        }

        outProperties[propertyName] = Berta::Dimension{ rawValue, unit };
    }

    std::unique_ptr<LayoutNode> LayoutParser::ParseNode()
    {
        if (!Accept(Token::Type::OpenBrace))
        {
            return nullptr;
        }

        std::string_view identifier;
        std::vector<std::unique_ptr<LayoutNode>> children;
        bool isVertical = false;
        Token::Type dockType = Token::Type::None;
        std::map<std::string, PropertyValue, std::less<>> properties; // Optimizada con less<> para string_view heterogéneo

        // Estado para saber qué tipo de nodo estamos construyendo
        bool isContainerKeywordExplicit = false;

        while (!IsAtEnd() && Peek().type != Token::Type::CloseBrace)
        {
            switch (Peek().type)
            {
            case Token::Type::Identifier:
                {
                    identifier = Peek().value; 
                    Advance();
                    break;
                }
            case Token::Type::VerticalLayout:
                isVertical = true;
                isContainerKeywordExplicit = true;
                Advance();
                break;

            case Token::Type::HorizontalLayout:
                isVertical = false;
                isContainerKeywordExplicit = true;
                Advance();
                break;

            case Token::Type::Dock:
                dockType = Token::Type::Dock;
                Advance();
                break;

            case Token::Type::DockPane:
                dockType = Token::Type::DockPane;
                Advance();
                break;

            case Token::Type::OpenBrace:
                {
                    if (auto child = ParseNode())
                    {
                        children.emplace_back(std::move(child));
                    }
                    break;
                }

            case Token::Type::Width:
            case Token::Type::Height:
            case Token::Type::MinHeight:
            case Token::Type::MaxHeight:
            case Token::Type::MinWidth:
            case Token::Type::MaxWidth:
                {
                    ParseProperty(properties);
                    break;
                }

            default:
                Advance();
                break;
            }

            if (Accept(Token::Type::Splitter))
            {
                auto splitterNode = std::make_unique<SplitterLayoutNode>(false);
                children.emplace_back(std::move(splitterNode));
            }
        }

        Expect(Token::Type::CloseBrace);

        std::unique_ptr<LayoutNode> node;

        if (dockType == Token::Type::Dock)
        {
            node = std::make_unique<DockLayoutNode>();
        }
        else if (dockType == Token::Type::DockPane)
        {
            node = std::make_unique<DockPaneLayoutNode>();
        }
        else if (children.empty() && !isContainerKeywordExplicit)
        {
            node = std::make_unique<LeafLayoutNode>();
        }
        else
        {
            node = std::make_unique<ContainerLayoutNode>(isVertical);
        }

        for (size_t i = 0; i < children.size(); ++i)
        {
            auto child = children[i].get();
            child->SetParentNode(node.get());
        
            if (i < children.size() - 1)
            {
                child->SetNext(children[i + 1].get());
            }
            if (i > 0)
            {
                child->SetPrev(children[i - 1].get());
            }

            if (child->GetType() == LayoutNodeType::Splitter)
            {
                auto splitterNode = static_cast<SplitterLayoutNode*>(child);
                splitterNode->SetOrientation(isVertical); // Sincroniza el flag interno

                if (isVertical)
                {
                    if (auto* dim = child->TryGetProperty<Berta::Dimension>("Width"))
                    {
                        // Movemos la estructura al nuevo eje sin alocaciones dinámicas
                        child->SetProperty("Height", *dim);
                        child->RemoveProperty("Width");
                    }
                }
            }
        }

        node->SetId(std::string(identifier));
        node->m_properties.swap(properties);
        node->m_children.swap(children);

        return node;
    }

    bool LayoutParser::IsAtEnd() const
    {
        return m_currentIndex >= m_tokens.size() || m_tokens[m_currentIndex].type == Token::Type::EndOfStream;
    }

    const Token& LayoutParser::Peek() const
    {
        if (m_currentIndex >= m_tokens.size())
        {
            return LayoutParser::g_endTokenType;
        }
        
        return m_tokens[m_currentIndex];
    }

    const Token& LayoutParser::Advance()
    {
        if (!IsAtEnd())
        {
            m_currentIndex++;
        }
        
        return m_tokens[m_currentIndex - 1];
    }
}
