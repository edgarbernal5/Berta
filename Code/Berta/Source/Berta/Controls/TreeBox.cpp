/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "TreeBox.h"

#include "Berta/GUI/Interface.h"
#include "Berta/GUI/EnumTypes.h"

#include <numeric>
#include <stack>

namespace Berta
{
	void TreeBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
		m_module.m_window = control.Handle();

		m_module.CalculateViewport(m_module.m_viewport);
	}

	void TreeBoxReactor::Update(Graphics& graphics)
	{
		BT_CORE_TRACE << " -- TreeBox Update() " << std::endl;
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();

		auto nodeHeight = window->ToScale(window->Appearance->ComboBoxItemHeight);

		graphics.DrawRectangle(window->Size.ToRectangle(), window->Appearance->BoxBackground, true);

		Point offset{ m_module.m_viewport.m_backgroundRect.X - m_module.m_scrollOffset.X, m_module.m_viewport.m_backgroundRect.Y - m_module.m_scrollOffset.Y };

		auto iconSize = m_module.m_window->ToScale(m_module.m_window->Appearance->SmallIconSize);
		for (auto& node : m_module.m_visibleNodes)
		{
			auto depth = m_module.CalculateNodeDepth(node);
			Rectangle nodeRect{ offset.X, offset.Y, m_module.m_viewport.m_backgroundRect.Width,nodeHeight };

			bool& isSelected = node->selected;
			offset.Y += static_cast<int>(nodeHeight);
		}

		if (m_module.m_viewport.m_needHorizontalScroll && m_module.m_viewport.m_needVerticalScroll)
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			graphics.DrawRectangle({ (int)(m_module.m_window->Size.Width - scrollSize) - 1, (int)(m_module.m_window->Size.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_window->Appearance->Background, true);
		}
		graphics.DrawRectangle(window->Size.ToRectangle(), enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
	}

	void TreeBoxReactor::Module::CalculateViewport(ViewportData& viewportData)
	{
		viewportData.m_backgroundRect = m_window->Size.ToRectangle();
		viewportData.m_backgroundRect.X = viewportData.m_backgroundRect.Y = 1;
		viewportData.m_backgroundRect.Width -= 2u;
		viewportData.m_backgroundRect.Height -= 2u;

		auto nodeHeight = m_window->ToScale(m_window->Appearance->ComboBoxItemHeight);
		viewportData.m_contentSize.Height = nodeHeight * CalculateTreeSize(m_root.firstChild);
		viewportData.m_contentSize.Width = viewportData.m_backgroundRect.Width;

		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		viewportData.m_needVerticalScroll = viewportData.m_contentSize.Height > viewportData.m_backgroundRect.Height;
		if (viewportData.m_needVerticalScroll)
		{
			viewportData.m_backgroundRect.Width -= scrollSize;
		}
	}

	void TreeBoxReactor::Module::CalculateVisibleNodes()
	{
		if (m_root.firstChild == nullptr)
		{
			m_visibleNodes.clear();
			return;
		}

		auto nodeHeight = m_window->ToScale(m_window->Appearance->ComboBoxItemHeight);
		auto visibleHeight = m_viewport.m_backgroundRect.Height;
		
		int firstVisibleNode = m_scrollOffset.Y / nodeHeight;
		int lastVisibleNode = (m_scrollOffset.Y + visibleHeight) / nodeHeight;

		int index = 0;

		std::stack<TreeNodeType*> stack;
		stack.push(&m_root);

		while (!stack.empty())
		{
			TreeNodeType* node = stack.top();
			stack.pop();

			if (index >= firstVisibleNode && index <= lastVisibleNode)
			{
				m_visibleNodes.emplace_back(node);
			}

			if (index > lastVisibleNode)
				break;

			++index;

			if (node->isExpanded && node->firstChild)
			{
				stack.push(node->firstChild);
			}

			if (node->nextSibling)
			{
				stack.push(node->nextSibling);
			}
		}
	}

	uint32_t TreeBoxReactor::Module::CalculateTreeSize(TreeNodeType* node)
	{
		if (node == nullptr)
		{
			return 0;
		}

		uint32_t size = 1;
		if (node->isExpanded)
		{
			for (TreeNodeType* child = node->firstChild; child; child = child->nextSibling)
			{
				size += CalculateTreeSize(child);
			}
		}
		return size;
	}

	uint32_t TreeBoxReactor::Module::CalculateNodeDepth(TreeNodeType* node)
	{
		uint32_t depth = 0;
		auto current = node->parent;
		while (current)
		{
			++depth;
			current = current->parent;
		}
		return depth;
	}

	void TreeBoxReactor::Module::Clear()
	{
		bool needUpdate = m_root.firstChild != nullptr;
		m_root.firstChild = nullptr;
		m_visibleNodes.clear();
		m_nodeLookup.clear();

		CalculateViewport(m_viewport);
		UpdateScrollBars();

		if (needUpdate)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	Berta::TreeBoxReactor::TreeNodeType* TreeBoxReactor::Module::GetNextVisible(TreeNodeType* node)
	{
		if (node == nullptr)
			return nullptr;

		if (node->isExpanded)
		{
			if (node->firstChild == nullptr)
				return nullptr;

			return node->firstChild;
		}

		if (node->nextSibling != nullptr)
		{
			return node->nextSibling;
		}

		auto parent = node->parent;
		if (parent == &m_root)
			return nullptr;

		//GetNextVisible(parent);

		return nullptr;
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const std::string& key, const std::string& text)
	{
		auto hasParentIndex = key.find_last_of('/');
		if (hasParentIndex != std::string::npos)
		{
			return Insert(key.substr(hasParentIndex, key.size()), text, key.substr(0, hasParentIndex));
		}

		auto node = std::make_unique<TreeNodeType>(key, text, &m_root);
		TreeNodeType* nodePtr = node.get();

		m_root.firstChild = nodePtr;
		
		m_nodeLookup[key] = std::move(node);
		return { nodePtr };
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const std::string& key, const std::string& text, const TreeNodeHandle& parentHandle)
	{
		TreeNodeType* parentNode{ nullptr };
		if (!parentHandle.empty())
		{
			auto it = m_nodeLookup.find(parentHandle);
			if (it == m_nodeLookup.end())
			{
				return {};
			}
			parentNode = it->second.get();
		}

		std::string handle = GenerateUniqueHandle(key, parentNode);
		auto node = std::make_unique<TreeNodeType>(handle, text, parentNode);
		node->parent = parentNode;
		TreeNodeType* nodePtr = node.get();

		if (parentNode)
		{
			if (parentNode->firstChild == nullptr)
			{
				parentNode->firstChild = nodePtr;
			}
			else
			{
				auto where = parentNode->firstChild;
				while (where->nextSibling != nullptr)
				{
					where = where->nextSibling;
				}
				where->nextSibling = nodePtr;
			}
		}
		else
		{
			if (m_root.firstChild == nullptr)
			{
				m_root.firstChild = nodePtr;
			}
			else
			{
				auto where = m_root.firstChild;
				while (where->nextSibling != nullptr)
				{
					where = where->nextSibling;
				}
				m_root.nextSibling = nodePtr;
			}
		}

		m_nodeLookup[handle] = std::move(node);
		return { nodePtr };
	}

	TreeBoxItem TreeBoxReactor::Module::Find(const TreeNodeHandle& handle)
	{
		if (handle.empty())
		{
			return {};
		}

		auto it = m_nodeLookup.find(handle);
		if (it == m_nodeLookup.end())
		{
			return {};
		}

		return { it->second.get() };
	}

	TreeNodeHandle TreeBoxReactor::Module::GenerateUniqueHandle(const std::string& key, TreeNodeType* parentNode)
	{
		if (parentNode)
		{
			return parentNode->key + "/" + key;
		}
		return key;
	}

	void TreeBoxReactor::Module::Erase(const TreeNodeHandle& handle)
	{
	}

	bool TreeBoxReactor::Module::UpdateScrollBars()
	{
		auto scrollSize = m_window->ToScale(m_window->Appearance->ScrollBarSize);
		bool needUpdate = false;
		
		if (m_viewport.m_needVerticalScroll)
		{
			Rectangle scrollRect{ static_cast<int>(m_window->Size.Width - scrollSize) - 1, 1, scrollSize, m_window->Size.Height - 2u };
			if (m_viewport.m_needHorizontalScroll)
			{
				scrollRect.Height -= scrollSize;
			}

			if (!m_scrollBarVert)
			{
				m_scrollBarVert = std::make_unique<ScrollBar>(m_window, false, scrollRect);
				m_scrollBarVert->GetEvents().ValueChanged.Connect([this](const ArgScrollBar& args)
					{
						m_scrollOffset.Y = args.Value;
						CalculateVisibleNodes();

						GUI::UpdateWindow(m_window);
					});
			}
			else
			{
				GUI::MoveWindow(m_scrollBarVert->Handle(), scrollRect);
			}

			auto nodeItemHeight = m_window->ToScale(m_window->Appearance->ComboBoxItemHeight);
			m_scrollBarVert->SetMinMax(0, (int)(m_viewport.m_contentSize.Height - m_viewport.m_backgroundRect.Height));
			m_scrollBarVert->SetPageStepValue(m_viewport.m_backgroundRect.Height);
			m_scrollBarVert->SetStepValue(nodeItemHeight);

			m_scrollOffset.Y = m_scrollBarVert->GetValue();
			CalculateVisibleNodes();
			needUpdate = true;
		}
		return needUpdate;
	}

	TreeBox::TreeBox(Window* parent, const Rectangle& rectangle)
	{
		Create(parent, true, rectangle);

#if BT_DEBUG
		m_handle->Name = "TreeBox";
#endif
	}

	void TreeBox::Clear()
	{
		m_reactor.GetModule().Clear();
	}

	TreeBoxItem TreeBox::Insert(const std::string& key, const std::string& text)
	{
		return m_reactor.GetModule().Insert(key, text);
	}
}
