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
	LayoutNode::LayoutNode(Type type) :
		m_type(type)
	{
	}

	void LayoutNode::AddWindow(Window* window)
	{
		m_controlContainer.AddWindow(window);
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

	void ContainerLayoutNode::Apply()
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

	void ContainerLayoutNode::CalculateAreas()
	{
		const auto& dpi = m_parentWindow->DPIScaleFactor;
		Number marginLeftNum = GetProperty<Number>("margin-left", {});
		Number marginRightNum = GetProperty<Number>("margin-right", {});
		Number marginTopNum = GetProperty<Number>("margin-top", {});
		Number marginBottomNum = GetProperty<Number>("margin-bottom", {});
		
		auto marginLeft = marginLeftNum.GetValue<int>(dpi);
		auto marginRight = marginRightNum.GetValue<int>(dpi);
		auto marginTop = marginTopNum.GetValue<int>(dpi);
		auto marginBottom = marginBottomNum.GetValue<int>(dpi);
		
		std::vector<bool> markedChildren(m_children.size(), false);
		std::vector<Rectangle> areas(m_children.size());

		auto parentArea = GetArea();
		auto remainArea = parentArea;

		int totalChildren = (int)m_children.size();
		auto fixedNodesCount = 0;

		for (size_t i = 0; i < m_children.size(); ++i)
		{
			auto& childNode = m_children[i];
			Rectangle childArea;

			if (m_isVertical)
				childArea.Width = parentArea.Width;
			else
				childArea.Height = parentArea.Height;

			auto dimensionType = m_isVertical ? "Height" : "Width";
			if (childNode->HasProperty<Number>(dimensionType))
			{
				auto dimensionNum = childNode->GetProperty<Number>(dimensionType);
				bool isPercentage = dimensionNum.isPercentage;
				uint32_t fixedSize = isPercentage ?
					static_cast<uint32_t>(dimensionNum.GetValue<double>() * (m_isVertical ? parentArea.Height : parentArea.Width) / 100.0) :
					static_cast<uint32_t>(dimensionNum.GetValue<int>(dpi));

				auto minDimensionType = m_isVertical ? "MinHeight" : "MinWidth";
				auto maxDimensionType = m_isVertical ? "MaxHeight" : "MaxWidth";

				if (childNode->HasProperty<Number>(minDimensionType))
				{
					auto dimensionNum = (uint32_t)childNode->GetProperty<Number>(minDimensionType).GetValue<int>(dpi);
					if (fixedSize < dimensionNum)
					{
						fixedSize = dimensionNum;
					}
				}
				if (childNode->HasProperty<Number>(maxDimensionType))
				{
					auto dimensionNum = (uint32_t)childNode->GetProperty<Number>(maxDimensionType).GetValue<int>(dpi);
					if (fixedSize > dimensionNum)
					{
						fixedSize = dimensionNum;
					}
				}

				if (m_isVertical)
				{
					childArea.Height = fixedSize;
				}
				else
				{
					childArea.Width = fixedSize;
				}

				if (m_isVertical)
				{
					remainArea.Height -= fixedSize;
				}
				else
				{
					remainArea.Width -= fixedSize;
				}

				markedChildren[i] = true;
				areas[i] = childArea;

				++fixedNodesCount;
			}
		}

		Point offset{ 0, 0 };
		int totalFreeCount = totalChildren - fixedNodesCount;
		
		for (size_t i = 0; i < m_children.size(); ++i)
		{
			auto& childNode = m_children[i];
			Rectangle childArea{ };
			childArea.X = marginLeft;
			childArea.Y = marginTop;
			if (markedChildren[i])
			{
				childArea.Width = areas[i].Width;
				childArea.Height = areas[i].Height;
			}
			else
			{
				childArea.Width = parentArea.Width;
				childArea.Height = parentArea.Height;
			}

			childArea.Width -= marginLeft;
			childArea.Height -= marginTop;

			if (!markedChildren[i])
			{
				if (m_isVertical)
				{
					auto part = remainArea.Height / totalFreeCount;
					childArea.Height = part;
					childArea.Y += offset.Y;
					offset.Y += part;
				}
				else
				{
					auto part = remainArea.Width / totalFreeCount;
					childArea.Width = part;
					childArea.X += offset.X;
					offset.X += part;
				}
				childArea.Width -= marginRight;
				childArea.Height -= marginBottom;
			}
			else {
				if (m_isVertical)
				{
					childArea.Y += offset.Y;
					offset.Y += areas[i].Height;
				}
				else
				{
					childArea.X += offset.X;
					offset.X += areas[i].Width;
				}
			}

			childNode->SetArea(childArea);

			childNode->CalculateAreas();
		}

		for (auto& childNode : m_children)
		{
			auto containerArea = childNode->GetArea();
			auto count = static_cast<uint32_t>(childNode->GetWindowsAreas().size());
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
					auto partHeight = containerArea.Height / count;
					childArea.Height = partHeight;
					offset2.Y = (int)partHeight;
				}
				else
				{
					auto partWidth = containerArea.Width / count;
					childArea.Width = partWidth;
					offset2.X = (int)partWidth;
				}
				windowArea.area = childArea;
				offset.X += offset2.X;
				offset.Y += offset2.Y;
			}
		}
	}

	ContainerLayoutNode::ContainerLayoutNode(bool isVertical) : 
		LayoutNode(Type::Container),
		m_isVertical(isVertical)
	{
	}

	void ContainerLayoutNode::AddChild(std::unique_ptr<LayoutNode>&& child)
	{
		m_children.push_back(std::move(child));
	}

	LeafLayoutNode::LeafLayoutNode() : 
		LayoutNode(Type::Leaf)
	{
	}
	void LeafLayoutNode::Apply()
	{
	}
	void LeafLayoutNode::CalculateAreas()
	{
	}
	SplitterLayoutNode::SplitterLayoutNode() :
		LayoutNode(Type::Splitter)
	{
	}
	void SplitterLayoutNode::Apply()
	{
	}
	void SplitterLayoutNode::CalculateAreas()
	{
	}
}