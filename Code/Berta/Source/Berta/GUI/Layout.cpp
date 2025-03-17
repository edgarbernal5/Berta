/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Layout.h"

#include "Berta/GUI/Control.h"
#include "Berta/GUI/Window.h"
#include "Berta/GUI/LayoutNodes.h"

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


	void Layout::AddPane(const std::string& paneId)
	{
		if (!m_rootNode)
			return;

		auto paneNode = m_rootNode->Find(paneId);
		if (!paneNode)
		{
			auto dockRoot = m_rootNode->FindFirst(LayoutNode::Type::Dock);
			if (!dockRoot || !dockRoot->m_children.empty())
			{
				return;
			}

			//create a new layout node (DockPaneLayoutNode)
			auto newPaneNode = std::make_unique<DockPaneLayoutNode>();

			m_dockPaneFields[paneId] = newPaneNode.get();
			auto& paneInfo = m_dockPaneInfoFields[paneId];
			paneInfo.id = paneId;

			newPaneNode->m_dockArea = std::make_unique<DockArea>();
			newPaneNode->m_dockArea->Create(m_parent, &paneInfo);

			dockRoot->m_children.emplace_back(std::move(newPaneNode));
		}
		else if (paneNode->GetType() == LayoutNode::Type::DockPane)
		{
			auto newPaneNode = reinterpret_cast<DockPaneLayoutNode*>(paneNode);

		}
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
			//TODO: add this same logic to visibility event?!
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

		auto rootNode = parser.Parse();
		if (!rootNode)
		{
			return;
		}
		BT_CORE_TRACE << "Parse completed." << std::endl;

		m_rootNode = std::move(rootNode);
		m_rootNode->SetParentWindow(m_parent);
		//m_rootNode->CalculateAreas();
	}

	Tokenizer::Tokenizer(const std::string& source) : 
		m_buffer(source.c_str()),
		m_bufferEnd(source.c_str() + source.size())
	{
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
		if (m_buffer[0] == '|')
		{
			m_token = Token::Type::Splitter;
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
		case '|':
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

	std::unique_ptr<LayoutNode> Layout::Parser::Parse()
	{
		std::string identifier;
		std::vector<std::unique_ptr<LayoutNode>> children;
		std::unique_ptr<LayoutNode> node;
		bool isVertical = false;
		Token::Type dockType = Token::Type::None;
		std::unordered_map<std::string, LayoutNode::PropertyValue> properties;

		m_tokenizer.Next();
		Token::Type currentToken = m_tokenizer.GetToken();
		while (currentToken != Token::Type::EndOfStream && currentToken != Token::Type::CloseBrace)
		{
			bool moveToNextToken = true;
			switch (currentToken)
			{
			case Berta::Token::Type::Identifier:
				identifier = m_tokenizer.GetIdentifier();
				break;
			case Berta::Token::Type::OpenBrace:
			{
				auto child = Parse();

				children.emplace_back(std::move(child));
				break;
			}
			case Berta::Token::Type::Splitter:
			{
				auto splitterNode = std::make_unique<SplitterLayoutNode>();
				splitterNode->SetProperty("Dimension", Number{ 4 });

				children.emplace_back(std::move(splitterNode));
				break;
			}
			case Berta::Token::Type::VerticalLayout:
				isVertical = true;
				break;
			case Berta::Token::Type::HorizontalLayout:
				isVertical = false;
				break;
			case Berta::Token::Type::Dock:
				dockType = Berta::Token::Type::Dock;
				break;
			case Berta::Token::Type::DockPane:
				dockType = Berta::Token::Type::DockPane;
				break;
			case Berta::Token::Type::Width:
			case Berta::Token::Type::Height:
			{
				auto propertyName = currentToken == Berta::Token::Type::Width ? "Width" : "Height";
				
				m_tokenizer.Next();
				currentToken = m_tokenizer.GetToken();
				if (!Expect(Token::Type::Equal))
				{
					return nullptr;
				}
				bool isNumberInt = false;
				bool isNumberDouble = false;
				if ((isNumberInt = Accept(Token::Type::NumberInt)) || (isNumberDouble = Accept(Token::Type::NumberDouble)))
				{
					Number number;
					if (isNumberInt)
					{
						number.SetValue(m_tokenizer.GetInt());
					}
					else
					{
						number.SetValue(m_tokenizer.GetDouble());
					}

					if (Accept(Token::Type::Percentage))
					{
						number.isPercentage = true;
					}
					
					properties[propertyName] = number;
					moveToNextToken = !IsEqualTo(Token::Type::CloseBrace);
				}
				break;
			}
			case Berta::Token::Type::MinHeight:
			case Berta::Token::Type::MaxHeight:
			case Berta::Token::Type::MinWidth:
			case Berta::Token::Type::MaxWidth:
			{
				m_tokenizer.Next();
				currentToken = m_tokenizer.GetToken();
				if (!Expect(Token::Type::Equal))
				{
					return nullptr;
				}

				if (IsEqualTo(Token::Type::NumberInt))
				{
					Number number;
					number.SetValue(m_tokenizer.GetInt());

					auto propertyName = currentToken == Berta::Token::Type::MinHeight ? "MinHeight" : 
						(currentToken == Berta::Token::Type::MaxHeight ? "MaxHeight" : 
							(currentToken == Berta::Token::Type::MinWidth ? "MinWidth" : "MaxWidth"));

					properties[propertyName] = number;
				}
				else
				{
					return nullptr;
				}
				break;
			}
			}
			if (moveToNextToken)
			{
				m_tokenizer.Next();
			}
			currentToken = m_tokenizer.GetToken();
		}

		switch (currentToken)
		{
		case Token::Type::CloseBrace:
		case Token::Type::EndOfStream:
			if (dockType == Berta::Token::Type::Dock)
			{
				node = std::make_unique<DockLayoutNode>();
			}
			else if (dockType == Berta::Token::Type::DockPane)
			{
				node = std::make_unique<DockPaneLayoutNode>();
			}
			else if (children.empty())
			{
				node = std::make_unique<LeafLayoutNode>();
			}
			else
			{
				node = std::make_unique<ContainerLayoutNode>(isVertical);
			}
			break;
		}

		for (size_t i = 0; i < children.size(); i++)
		{
			auto child = children[i].get();
			child->SetParentNode(node.get());
			if (i < children.size() - 1)
			{
				child->SetNext(children[i + 1].get());
			}
			if (i > 0)
			{
				child->SetPrev(children[i- 1].get());
			}

			if (child->GetType() == LayoutNode::Type::Splitter)
			{
				auto dimension = child->GetProperty<Number>("Dimension");
				child->SetProperty(isVertical ? "Height" : "Width", dimension);
				child->RemoveProperty("Dimension");
			}
		}

		node->SetId(identifier);
		node->m_properties.swap(properties);
		node->m_children.swap(children);

		return node;
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

	bool Layout::Parser::IsEqualTo(Token::Type tokenId)
	{
		return m_tokenizer.GetToken() == tokenId;
	}

	void LayoutControlContainer::AddWindow(Window* window)
	{
		m_windows.push_back({ window });
	}
}