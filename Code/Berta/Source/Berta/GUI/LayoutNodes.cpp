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
			{
				childArea.Width = parentArea.Width;
				childNode->m_fixedWidth.Reset();
			}
			else
			{
				childArea.Height = parentArea.Height;
				childNode->m_fixedHeight.Reset();
			}

			auto dimensionType = m_isVertical ? "Height" : "Width";
			if (childNode->HasProperty<Number>(dimensionType))
			{
				auto dimensionNum = childNode->GetProperty<Number>(dimensionType);
				bool isPercentage = dimensionNum.isPercentage;
				uint32_t fixedSize = isPercentage ?
					static_cast<uint32_t>(dimensionNum.GetValue<double>() * (m_isVertical ? parentArea.Height : parentArea.Width)) :
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
					childNode->m_fixedHeight.SetValue((int)fixedSize);
					remainArea.Height -= fixedSize;
				}
				else
				{
					childArea.Width = fixedSize;
					childNode->m_fixedWidth.SetValue((int)fixedSize);
					remainArea.Width -= fixedSize;
				}

				markedChildren[i] = true;
				areas[i] = childArea;

				++fixedNodesCount;
			}
		}

		Point offset{ 0, 0 };
		int totalFreeCount = totalChildren - fixedNodesCount;
		uint32_t appliedPixels = 0;
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
					uint32_t part = 0u;
					if (!childNode->m_fixedHeight.HasValue())
					{
						childNode->m_fixedHeight.isPercentage = true;
						childNode->m_fixedHeight.SetValue(1.0 / totalFreeCount);
						part = remainArea.Height / totalFreeCount;
					}
					else
					{
						if (childNode->m_fixedHeight.isPercentage)
						{
							auto partDouble = childNode->m_fixedHeight.GetValue<double>() * remainArea.Height;
							part = (uint32_t)(partDouble);
						}
						else
						{
							part = childNode->m_fixedHeight.GetValue<uint32_t>();
						}
					}
					appliedPixels += part;

					childArea.Height = part;
					childArea.Y += offset.Y;
					offset.Y += part;
				}
				else
				{
					uint32_t part = 0u;
					if (!childNode->m_fixedWidth.HasValue())
					{
						childNode->m_fixedWidth.isPercentage = true;
						childNode->m_fixedWidth.SetValue(1.0 / totalFreeCount);
						part = remainArea.Width / totalFreeCount;
					}
					else
					{
						if (childNode->m_fixedWidth.isPercentage)
						{
							auto partDouble = childNode->m_fixedWidth.GetValue<double>() * remainArea.Width;
							part = (uint32_t)(partDouble);
						}
						else
						{
							part = childNode->m_fixedWidth.GetValue<uint32_t>();
						}
					}
					appliedPixels += part;

					childArea.Width = part;
					childArea.X += offset.X;
					offset.X += part;
				}
				childArea.Width -= marginRight;
				childArea.Height -= marginBottom;
			}
			else
			{
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

			if (i == m_children.size() - 1)
			{
				if (m_isVertical && remainArea.Height > appliedPixels)
				{
					childArea.Height += remainArea.Height - appliedPixels;
				}
				else if (!m_isVertical && remainArea.Width > appliedPixels)
				{
					childArea.Width += remainArea.Width - appliedPixels;
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
				
				windowArea.area = childArea;
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
		m_children.emplace_back(std::move(child));
	}

	LeafLayoutNode::LeafLayoutNode() : 
		LayoutNode(Type::Leaf)
	{
	}

	void LeafLayoutNode::CalculateAreas()
	{
		//auto parentArea = GetParentNode()->GetArea();

	}

	SplitterLayoutNode::SplitterLayoutNode() :
		LayoutNode(Type::Splitter)
	{
	}

	void SplitterLayoutNode::CalculateAreas()
	{
		auto splitterArea = GetArea();
		if (!m_splitter)
		{
			m_splitter = std::make_unique<SplitterLayoutControl>(m_parentWindow, splitterArea);
			m_splitter->GetEvents().MouseDown.Connect([this](const ArgMouse& args)
			{
				if (!args.ButtonState.LeftButton)
					return;

				GUI::Capture(m_splitter->Handle());

				m_splitterBeginRect = m_splitter->GetArea();
				m_mousePositionOffset = -args.Position;

				m_leftArea = m_prevNode->GetArea();
				m_rightArea = m_nextNode->GetArea();

				BT_CORE_TRACE << " * begin left area = " << m_leftArea << std::endl;
				BT_CORE_TRACE << " * begin right area = " << m_rightArea << std::endl;
				BT_CORE_TRACE << " * m_splitterBeginRect = " << m_splitterBeginRect << std::endl;
				m_isSplitterMoving = true;
				auto containerNode = reinterpret_cast<ContainerLayoutNode*>(m_parentNode);
				
				m_isVertical = containerNode->GetOrientation();
			});

			m_splitter->GetEvents().MouseMove.Connect([this, splitterArea](const ArgMouse& args)
			{
				if (!m_isSplitterMoving)
					return;
				
				auto delta = GUI::GetAbsolutePosition(m_splitter->Handle()) + args.Position - m_splitterBeginRect + m_mousePositionOffset;
				if (m_isVertical)
				{

				}
				else
				{
					auto deltaX = delta.X;
					auto newLeftArea = m_leftArea;
					auto newRightArea = m_rightArea;

					int leftLimit = (std::max)(0, (int)newLeftArea.Width + deltaX);
					newLeftArea.Width = (uint32_t)leftLimit;

					newRightArea.X = (std::max)(newLeftArea.X, newRightArea.X + deltaX);
					int rightLimit = (std::max)(0, (int)newRightArea.Width - deltaX);
					newRightArea.Width = (uint32_t)rightLimit;

					auto containerNode = reinterpret_cast<ContainerLayoutNode*>(m_parentNode);

					BT_CORE_TRACE << " * left area = " << newLeftArea << std::endl;
					BT_CORE_TRACE << " * right area = " << newRightArea << std::endl;
					auto parentArea = containerNode->GetArea();
					m_prevNode->SetAreaWithPercentage(newLeftArea, parentArea, splitterArea);
					m_nextNode->SetAreaWithPercentage(newRightArea, parentArea, splitterArea);

					auto newSplitterArea = splitterArea;
					newSplitterArea.X += deltaX;

					BT_CORE_TRACE << " - * newSplitterArea = " << newSplitterArea << std::endl;
					SetArea(newSplitterArea);
					GUI::MoveWindow(m_splitter->Handle(), newSplitterArea);

					containerNode->CalculateAreas();
					containerNode->Apply();

					GUI::UpdateTree(containerNode->GetParentWindow());//TODO: no se si tengamos que hacer esta llamada aca. es probable que la tenga que hacer el MoveWindow or ResizeWindow
					GUI::UpdateWindow(containerNode->GetParentWindow());//TODO: no se si tengamos que hacer esta llamada aca. es probable que la tenga que hacer el MoveWindow or ResizeWindow
				}
			});

			m_splitter->GetEvents().MouseUp.Connect([this](const ArgMouse& args)
			{
				if (!m_isSplitterMoving)
					return;

				m_isSplitterMoving = false;
				GUI::ReleaseCapture(m_splitter->Handle());
			});
		}
		else
		{
			GUI::MoveWindow(m_splitter->Handle(), splitterArea);
		}
	}

	SplitterLayoutControl::SplitterLayoutControl(Window* parent, const Rectangle& rectangle, bool visible) :
		Panel(parent, rectangle, visible)
	{
#if BT_DEBUG
		m_handle->Name = "SplitterLayoutControl";
#endif
	}
}