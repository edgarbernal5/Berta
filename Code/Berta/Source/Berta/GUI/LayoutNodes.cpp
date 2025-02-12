/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "LayoutNodes.h"

#include "Berta/GUI/Interface.h"

namespace Berta
{
	LayoutNode::LayoutNode(const std::string& id) : 
		m_id(id)
	{
	}

	LayoutNode::LayoutNode(bool isVertical) : m_isVertical(isVertical)
	{
	}

	void LayoutNode::AddWindow(Window* window)
	{
		m_controlContainer.AddWindow(window);
	}

	void LayoutNode::AddChild(std::unique_ptr<LayoutNode>&& child)
	{
		m_children.push_back(std::move(child));
	}

	LayoutNode* LayoutNode::Find(const std::string& id)
	{
		return Find(id, this);
	}

	LayoutNode* LayoutNode::Find(const std::string& id, LayoutNode* node)
	{
		if (node->GetId() == id)
			return node;

		for (auto& childPtr : node->m_children)
		{
			auto child = Find(id, childPtr.get());
			if (child)
			{
				return child;
			}
		}

		return nullptr;
	}

	void LayoutNode::Apply()
	{
		for (auto& childNode : m_children)
		{
			childNode->Apply();
			for (auto& windowArea : childNode->GetWindowsAreas())
			{
				GUI::MoveWindow(windowArea.window, windowArea.area);
			}
		}
	}

	void LayoutNode::CalculateAreas()
	{
		auto parentArea = GetArea();

		int count = m_children.size();
		Point offset{};
		for (auto& childNode : m_children)
		{
			auto childArea = childNode->GetArea();
			childArea.Height = parentArea.Height;
			childArea.Width = parentArea.Width;
			if (m_isVertical)
			{
				auto part = parentArea.Height / count;
				childArea.Height = part;
				childArea.Y = offset.Y;
				offset.Y += parentArea.Height;
			}
			else
			{
				auto part = parentArea.Width / count;
				childArea.Width = part;
				childArea.X = offset.X;
				offset.X += parentArea.Width;
			}
			childNode->SetArea(childArea);

			childNode->CalculateAreas();
		}

		for (auto& childNode : m_children)
		{
			auto containerArea = childNode->GetArea();
			auto count = (uint32_t)childNode->GetWindowsAreas().size();
			Point offset{ containerArea.X, containerArea.Y };
			for (auto& windowArea : childNode->GetWindowsAreas())
			{
				Rectangle childArea{};
				childArea.X = offset.X;
				childArea.Y = offset.Y;
				childArea.Height = containerArea.Height;
				childArea.Width = containerArea.Width;
				Point offset2{};
				if (m_isVertical)
				{
					auto part = containerArea.Height / count;
					childArea.Height = part;
					offset2.Y = (int)part;
				}
				else
				{
					auto part = containerArea.Width / count;
					childArea.Width = part;
					offset2.X = (int)part;
				}
				windowArea.area = childArea;
				offset.X += offset2.X;
				offset.Y += offset2.Y;
			}
		}
	}
}