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
		const auto& dpi = m_parentWindow->DPIScaleFactor;
		Number marginLeftNum = GetProperty<Number>("margin-left", {});
		Number marginRightNum = GetProperty<Number>("margin-right", {});
		Number marginTopNum = GetProperty<Number>("margin-top", {});
		Number marginBottomNum = GetProperty<Number>("margin-bottom", {});
		
		auto marginLeft = marginLeftNum.GetValue<int>(dpi);
		auto marginRight = marginRightNum.GetValue<int>(dpi);
		auto marginTop = marginTopNum.GetValue<int>(dpi);
		auto marginBottom = marginBottomNum.GetValue<int>(dpi);
		
		int count = (int)m_children.size();
		
		Point offset{ 0, 0 };
		std::vector<bool> mark(m_children.size(), false);
		std::vector<Rectangle> areas(m_children.size());

		auto parentArea = GetArea();
		auto remainArea = parentArea;
		auto fixedNodesCount = 0;

		for (size_t i = 0; i < m_children.size(); ++i)
		{
			auto& childNode = m_children[i];
			Rectangle childArea;

			if (m_isVertical)
			{
				childArea.Width = parentArea.Width;
				if (HasProperty<Number>("Height"))
				{
					auto heightNum = GetProperty<Number>("Height");
					if (heightNum.isPercentage)
					{
						auto fixedHeight = static_cast<uint32_t>(heightNum.GetValue<double>(dpi) * parentArea.Height);
						remainArea.Height -= fixedHeight;
						childArea.Height = static_cast<uint32_t>(fixedHeight);
					}
					else
					{
						auto fixedHeight = static_cast<uint32_t>(heightNum.GetValue<int>(dpi));
						remainArea.Height -= fixedHeight;
						childArea.Height = fixedHeight;
					}
					mark[i] = true;
					areas[i] = childArea;
					++fixedNodesCount;
				}
			}
			else
			{
				childArea.Height = parentArea.Height;
				if (HasProperty<Number>("Width"))
				{
					auto widthNum = GetProperty<Number>("Width");
					if (widthNum.isPercentage)
					{
						auto fixedWidth = static_cast<uint32_t>(widthNum.GetValue<double>(dpi) * parentArea.Width);
						remainArea.Width -= fixedWidth;
						childArea.Width = static_cast<uint32_t>(fixedWidth);
					}
					else
					{
						auto fixedWidth = static_cast<uint32_t>(widthNum.GetValue<int>(dpi));
						remainArea.Width -= fixedWidth;
						childArea.Width = fixedWidth;
					}
					mark[i] = true;
					areas[i] = childArea;
					++fixedNodesCount;
				}
			}
		}
		int totalCount = count - fixedNodesCount;
		
		for (size_t i = 0; i < m_children.size(); ++i)
		{
			auto& childNode = m_children[i];
			Rectangle childArea{ };
			childArea.X = marginLeft;
			childArea.Y = marginTop;
			if (mark[i])
			{
				childArea.Width = areas[i].Width;
				childArea.Height = areas[i].Height;
			}
			else
			{
				childArea.Width = parentArea.Width;
				childArea.Height = parentArea.Height;
			}
			//Rectangle childArea{ marginLeft, marginTop, parentArea.Width, parentArea.Height };

			childArea.Width -= marginLeft;
			childArea.Height -= marginTop;

			if (!mark[i])
			{
				if (m_isVertical)
				{
					auto part = remainArea.Height / totalCount;
					childArea.Height = part;
					childArea.Y += offset.Y;
					offset.Y += part;
				}
				else
				{
					auto part = remainArea.Width / totalCount;
					childArea.Width = part;
					childArea.X += offset.X;
					offset.X += part;
				}
				childArea.Width -= marginRight;
				childArea.Height -= marginBottom;
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