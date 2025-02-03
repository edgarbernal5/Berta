/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "LayoutNodes.h"

namespace Berta
{
	LayoutNode::LayoutNode(const std::string& id) : 
		m_id(id)
	{
	}

	ContainerLayout::ContainerLayout(bool isVertical) :
		m_isVertical(isVertical)
	{
	}

	void ContainerLayout::AddChild(std::unique_ptr<LayoutNode>&& child)
	{
		m_children.push_back(std::move(child));
	}

	void ContainerLayout::Apply()
	{
		for (auto& childNode : m_children)
		{

		}
	}

	void ContainerLayout::CalculateAreas()
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
				offset.Y += part + 1;
			}
			else
			{
				auto part = parentArea.Width / count;
				childArea.Width = part;
				childArea.X = offset.X;
				offset.X += part + 1;
			}
			childNode->SetArea(childArea);

			childNode->CalculateAreas();
		}
	}
}