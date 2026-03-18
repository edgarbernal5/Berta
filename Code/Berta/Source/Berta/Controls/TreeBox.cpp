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
	/*TreeNodeType* TreeNodeType::Add(const TreeNodeHandle& childKey, const std::string& text_, TreeNodeType* parent_)
	{
		auto it = m_lookup.find(childKey);
		if (it != m_lookup.end())
		{
			return it->second.get();
		}

		auto child = std::make_unique<TreeNodeType>(childKey, text_, parent_);
		TreeNodeType* childPtr = child.get();
		m_lookup[childKey] = std::move(child);
		
		parent_->children.emplace_back(childPtr);

		return childPtr;
	}
	
	
	TreeNodeType* TreeNodeType::Find(const TreeNodeHandle& key)
	{
		auto it = m_lookup.find(key);
		return it != m_lookup.end() ? it->second.get() : nullptr;
	}*/

	void TreeBoxReactor::Init(ControlBase& control, Graphics* graphics)
	{
		m_control = &control;
		m_module.m_window = control.Handle();
		m_module.m_control = m_control;
		m_module.m_graphics = graphics;
		
		m_module.InitScrollableView();
	}

	void TreeBoxReactor::Update(Graphics& graphics)
	{
		//BT_CORE_TRACE << " -- TreeBox Update() " << std::endl;
		auto window = m_control->Handle();
		bool enabled = m_control->GetEnabled();
		
		auto globalRect = window->ClientSize.ToRectangle();
		graphics.DrawRectangle(globalRect, window->Appearance->BoxBackground, true);
		
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_module.m_window->Appearance.get());
		if (m_module.m_needsRecalculate)
		{
			auto nodeHeight = static_cast<int>(window->ToScale(appearance->TreeItemHeight));
			int maxWidth = m_module.m_visibleWidths.empty() ? 0 : *m_module.m_visibleWidths.rbegin();
        
			m_module.m_scrollableView->SetContentSize(Size{ (uint32_t)maxWidth, static_cast<uint32_t>(m_module.m_flatVisibleTree.size()) * nodeHeight });
			m_module.m_needsRecalculate = false;
		}
		
		m_module.DrawTreeNodes(graphics);

		if (m_module.m_scrollableView->HasVerticalScroll() && m_module.m_scrollableView->HasHorizontalScroll())
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			graphics.DrawRectangle({ (int)(m_module.m_window->ClientSize.Width - scrollSize) - 1, (int)(m_module.m_window->ClientSize.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_window->Appearance->Background, true);
		}
		graphics.DrawRectangle(globalRect, enabled ? window->Appearance->BoxBorderColor : window->Appearance->BoxBorderDisabledColor, false);
	}

	void TreeBoxReactor::Resize(Graphics& graphics, const ArgResize& args)
	{
		m_module.UpdateScrollData();
	}

	void TreeBoxReactor::DblClick(Graphics& graphics, const ArgMouse& args)
	{		
		if (m_module.m_flatVisibleTree.empty())
		{
			return;
		}

		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_module.m_window->Appearance.get());
		auto nodeHeight = static_cast<int>(m_module.m_window->ToScale(appearance->TreeItemHeight));
		int scrollY = m_module.m_scrollableView->GetScrollOffset().Y;
    
		size_t clickedIndex = (args.Position.Y + scrollY) / nodeHeight;
		if (clickedIndex >= m_module.m_flatVisibleTree.size())
		{
			return;
		}

		TreeNodeType* clickedNode = m_module.m_flatVisibleTree[clickedIndex].Node;
		if (!clickedNode->children.empty())
		{
			clickedNode->isExpanded = !clickedNode->isExpanded;
			m_module.RebuildFlatTree();
		}
		
		GUI::MarkAsNeedUpdate(m_module.m_window);
	}

	void TreeBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_flatVisibleTree.empty())
		{
			return;
		}
		
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_module.m_window->Appearance.get());
		
		auto clientArea = m_module.m_scrollableView->GetClientArea();
		auto nodeHeight = static_cast<int>(m_module.m_window->ToScale(appearance->TreeItemHeight));
		int scrollX = m_module.m_scrollableView->GetScrollOffset().X;
		int scrollY = m_module.m_scrollableView->GetScrollOffset().Y;

		int absoluteY = args.Position.Y + scrollY - clientArea.Y;
		size_t clickedIndex = absoluteY / nodeHeight;

		if (clickedIndex >= m_module.m_flatVisibleTree.size())
		{
			return;
		}

		const auto& flatNode = m_module.m_flatVisibleTree[clickedIndex];
		TreeNodeType* clickedNode = flatNode.Node;

		auto depthMultiplier = static_cast<int>(m_module.m_window->ToScale(appearance->DepthWidthMultiplier));
		auto expanderSize = m_module.m_window->ToScale(appearance->ExpanderButtonSize);
		
		int expanderMarginX = static_cast<int>(depthMultiplier - expanderSize) >> 1;
		int expanderStartX = (flatNode.Level * depthMultiplier) - scrollX + clientArea.X + expanderMarginX;
		int expanderEndX = expanderStartX + (int)expanderSize;

		if (!clickedNode->children.empty() && args.Position.X >= expanderStartX && args.Position.X <= expanderEndX)
		{
			clickedNode->isExpanded = !clickedNode->isExpanded;
	        
			m_module.EmitExpansionEvent(clickedNode);
			
			m_module.RebuildFlatTree();
			
			GUI::MarkAsNeedUpdate(m_module.m_window);
			return;
		}

		bool selectionChanged = m_module.m_selectionController.Select
		(
			clickedNode,
			m_module.m_ctrlPressed,
			m_module.m_shiftPressed,
			m_module.m_treeRangeResolver
		);

		if (selectionChanged)
		{
			m_module.m_focusedNode = clickedNode;
			m_module.m_needsRepaint = true;

			GUI::MarkAsNeedUpdate(m_module.m_window);
			// Disparar evento de selección de la librería
			// ArgTreeBoxSelection eventArgs;
			// eventArgs.Items = Convertir m_selectionController.GetSelectedItems() a TreeBoxItem
			// m_control->Events.Selected(eventArgs);
		}
		
		if (m_module.m_allowDragAndDrop && clickedNode != nullptr)
		{
			m_module.m_draggedNode = clickedNode;
			m_module.m_dragStartPoint = args.Position;
			m_module.m_isDragging = false;
		}
	}

	void TreeBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_flatVisibleTree.empty())
		{
			return;
		}
		
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_module.m_window->Appearance.get());
		
		auto dragThreshold = m_module.m_window->ToScale(3);
		if (m_module.m_draggedNode && !m_module.m_isDragging)
		{
			if (std::abs(args.Position.X - m_module.m_dragStartPoint.X) > dragThreshold || 
				std::abs(args.Position.Y - m_module.m_dragStartPoint.Y) > dragThreshold)
			{
				m_module.m_isDragging = true;
			}
		}

		if (m_module.m_isDragging)
		{
			auto nodeHeight = static_cast<int>(m_module.m_window->ToScale(appearance->TreeItemHeight));
			int absoluteY = args.Position.Y + m_module.m_scrollableView->GetScrollOffset().Y;
			size_t hoveredIndex = absoluteY / nodeHeight;

			if (hoveredIndex < m_module.m_flatVisibleTree.size())
			{
				TreeNodeType* hoverNode = m_module.m_flatVisibleTree[hoveredIndex].Node;
            
				if (hoverNode != m_module.m_draggedNode && !m_module.IsDescendantOf(hoverNode, m_module.m_draggedNode))
				{
					m_module.m_dropTargetNode = hoverNode;
                
					// Calcular la zona de Drop
					int relativeY = absoluteY % nodeHeight;
					if (relativeY < nodeHeight / 4)
					{
						m_module.m_dropPosition = DropPosition::Before;
					}
					else if (relativeY > (nodeHeight * 3) / 4)
					{
						m_module.m_dropPosition = DropPosition::After;
					}
					else
					{
						m_module.m_dropPosition = DropPosition::Inside;
					}
				}
				else
				{
					m_module.m_dropTargetNode = nullptr;
					m_module.m_dropPosition = DropPosition::None;
				}
			}
        
			m_module.m_needsRepaint = true;
			GUI::MarkAsNeedUpdate(m_module.m_window);
			return;
		}
		
		auto nodeHeight = static_cast<int>(m_module.m_window->ToScale(appearance->TreeItemHeight));
		int scrollY = m_module.m_scrollableView->GetScrollOffset().Y;

		int absoluteY = args.Position.Y + scrollY;
		size_t hoveredIndex = absoluteY / nodeHeight;

		TreeNodeType* currentHover = nullptr;
		if (hoveredIndex < m_module.m_flatVisibleTree.size())
		{
			currentHover = m_module.m_flatVisibleTree[hoveredIndex].Node;
		}

		if (m_module.m_hoveredNode != currentHover)
		{
			m_module.m_hoveredNode = currentHover;
			m_module.m_needsRepaint = true;
        
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void TreeBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_isDragging)
		{
			if (m_module.m_dropTargetNode && m_module.m_dropPosition != DropPosition::None)
			{
				// Opcional: Podrías disparar un evento "OnBeforeDrop" aquí para que el usuario 
				// de la librería pueda cancelarlo si rompe alguna regla de negocio de su app.

				m_module.m_model.MoveNode(m_module.m_draggedNode, m_module.m_dropTargetNode, m_module.m_dropPosition);
            
				m_module.RebuildFlatTree(); 
				m_module.UpdateScrollData();
			}
		}
		
		if (m_module.m_draggedNode != nullptr)
		{
			m_module.m_isDragging = false;
			m_module.m_draggedNode = nullptr;
			m_module.m_dropTargetNode = nullptr;
			m_module.m_dropPosition = DropPosition::None;
			
			m_module.m_needsRepaint = true;
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void TreeBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_hoveredNode != nullptr)
		{
			m_module.m_hoveredNode = nullptr;
			m_module.m_needsRepaint = true;
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void TreeBoxReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
		m_module.m_scrollableView->HandleMouseWheel(args);
	}

	void TreeBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;
		
		if (m_module.m_flatVisibleTree.empty())
		{
			return;
		}
		
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_module.m_window->Appearance.get());
		auto nodeHeight = m_module.m_window->ToScale(appearance->TreeItemHeight);
		int currentIndex = -1;
		if (m_module.m_focusedNode)
		{
			auto it = std::find_if(m_module.m_flatVisibleTree.begin(), m_module.m_flatVisibleTree.end(),
								   [this](const FlatNode& fn) { return fn.Node == m_module.m_focusedNode; });
        
			if (it != m_module.m_flatVisibleTree.end())
			{
				currentIndex = static_cast<int>(std::distance(m_module.m_flatVisibleTree.begin(), it));
			}
		}

		if (currentIndex == -1)
		{
			currentIndex = 0;
			m_module.m_focusedNode = m_module.m_flatVisibleTree[0].Node;
		}

		int targetIndex = currentIndex;
		bool structureChanged = false;
		
		switch (args.Key)
		{
		case KeyboardKey::ArrowDown:
			targetIndex = std::min<int>(static_cast<int>(m_module.m_flatVisibleTree.size()) - 1, currentIndex + 1);
			break;

		case KeyboardKey::ArrowUp:
			targetIndex = std::max<int>(0, currentIndex - 1);
			break;

		case KeyboardKey::ArrowRight:
			if (!m_module.m_focusedNode->children.empty()) 
			{
				if (!m_module.m_focusedNode->isExpanded)
				{
					m_module.m_focusedNode->isExpanded = true;
					structureChanged = true;
				}
				else if (currentIndex + 1 < static_cast<int>(m_module.m_flatVisibleTree.size()))
				{
					targetIndex = currentIndex + 1;
				}
			}
			break;

		case KeyboardKey::ArrowLeft:
			if (m_module.m_focusedNode->isExpanded && !m_module.m_focusedNode->children.empty())
			{
				m_module.m_focusedNode->isExpanded = false;
				structureChanged = true;
			}
			else if (m_module.m_focusedNode->parent && m_module.m_focusedNode->parent != m_module.m_model.GetRoot())
			{
				auto it = std::find_if(m_module.m_flatVisibleTree.begin(), m_module.m_flatVisibleTree.end(),
					[this](const FlatNode& fn) { return fn.Node == m_module.m_focusedNode->parent; });
                    
				if (it != m_module.m_flatVisibleTree.end())
				{
					targetIndex = static_cast<int>(std::distance(m_module.m_flatVisibleTree.begin(), it));
				}
			}
			break;

		case KeyboardKey::Space:
		case KeyboardKey::Enter:
			m_module.m_selectionController.Select(m_module.m_focusedNode, m_module.m_ctrlPressed, m_module.m_shiftPressed, m_module.m_treeRangeResolver);
			m_module.m_needsRepaint = true;
			GUI::MarkAsNeedUpdate(m_module.m_window);
			return;

		case KeyboardKey::PageUp:
			{
				auto visibleNodesCount = (int)(m_module.m_scrollableView->GetClientArea().Height / nodeHeight);
				targetIndex = std::max<int>(0, currentIndex - visibleNodesCount);
			}
			break;
		case KeyboardKey::PageDown:
			{
				auto visibleNodesCount = (int)(m_module.m_scrollableView->GetClientArea().Height / nodeHeight);
				targetIndex = std::min<int>(static_cast<int>(m_module.m_flatVisibleTree.size()) - 1, currentIndex + visibleNodesCount);
			}
			break;
			
		case KeyboardKey::Home:
			targetIndex = 0;
			break;
		case KeyboardKey::End:
			targetIndex = static_cast<int>(m_module.m_flatVisibleTree.size()) - 1;
			break;
			
		default:
			return;
		}

		if (structureChanged)
		{
			m_module.EmitExpansionEvent(m_module.m_focusedNode);
			m_module.RebuildFlatTree();

			GUI::MarkAsNeedUpdate(m_module.m_window);
			return;
		}
		
		if (targetIndex != currentIndex)
		{
			m_module.m_focusedNode = m_module.m_flatVisibleTree[targetIndex].Node;

			bool selectionChanged = m_module.m_selectionController.Select
			(
				m_module.m_focusedNode, 
				m_module.m_ctrlPressed, 
				m_module.m_shiftPressed, 
				m_module.m_treeRangeResolver
			);

			if (selectionChanged)
			{
				// Disparar evento de selección:
				// m_control->Events.Selected( ... );
			}
			
			Rectangle targetBounds = { 0, targetIndex * static_cast<int>(nodeHeight), 100 , nodeHeight };
        
			bool scrollChanged = m_module.m_scrollableView->EnsureVisibility(targetBounds);
        
			m_module.m_needsRepaint = true;
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
	}

	void TreeBoxReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
	}
	
	void TreeBoxReactor::Module::Update()
	{
		UpdateScrollData();
	}

	void TreeBoxReactor::Module::Draw()
	{
		GUI::UpdateWindow(m_window);
	}

	void TreeBoxReactor::Module::DrawTreeNodes(Graphics& graphics)
	{
		if (m_flatVisibleTree.empty())
		{
			return;
		}
		
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		
		auto nodeHeight = static_cast<int>(m_window->ToScale(appearance->TreeItemHeight));
		auto expanderSize = m_window->ToScale(appearance->ExpanderButtonSize);
		auto depthMultiplier = static_cast<int>(m_window->ToScale(appearance->DepthWidthMultiplier));
		
		int expanderMarginX = static_cast<int>(depthMultiplier - expanderSize) >> 1;
		
		int scrollX = m_scrollableView->GetScrollOffset().X;
		int scrollY = m_scrollableView->GetScrollOffset().Y;
		auto clientArea = m_scrollableView->GetClientArea();
		int clientWidth = (int)clientArea.Width;
    
		size_t startIndex = std::max<size_t>(0ULL, static_cast<size_t>(scrollY / nodeHeight));
		size_t visibleCount = (clientArea.Height / nodeHeight) + 2; // +2 de margen
		size_t endIndex = std::min<size_t>(m_flatVisibleTree.size(), startIndex + visibleCount);

		for (size_t i = startIndex; i < endIndex; ++i)
		{
			const auto& flatNode = m_flatVisibleTree[i];
			const auto& node = flatNode.Node;
        
			int indentX = (flatNode.Level * depthMultiplier) - scrollX + clientArea.X;
			int drawY = static_cast<int>(i * nodeHeight) - scrollY + clientArea.Y;

			Rectangle rowRect{ clientArea.X, drawY, clientArea.Width, (uint32_t)nodeHeight };
			
			bool isSelected = m_selectionController.IsSelected(node);
			bool isFocused = (node == m_focusedNode);
			bool isHovered = (node == m_hoveredNode);
			
			if (isSelected)
			{
				graphics.DrawRectangle(rowRect, appearance->SelectionHighlightColor, true); 
			}
			else if (isHovered)
			{
				graphics.DrawRectangle(rowRect, appearance->HighlightColor, true); 
			}
			if (isFocused)
			{
				graphics.DrawRectangle(rowRect, appearance->Foreground, false);
			}
			Rectangle expanderRect{ indentX + expanderMarginX, drawY + (nodeHeight - (int)expanderSize) / 2, expanderSize, expanderSize };
			
			/*
			if (m_drawImages)
			{
				if (node->icon)
				{
					auto positionY = (nodeHeight - iconSize) >> 1;
					node->icon.Paste(graphics, { nodeRect.X + iconMargin, nodeRect.Y + (int)positionY, iconSize , iconSize });
				}
				contentOffsetX += iconSize + iconMargin * 2;
			}
			 */
			if (!node->children.empty())
			{
				int arrowWidth = m_window->ToScale(4);
				int arrowLength = m_window->ToScale(2);
				graphics.DrawRoundRectBox(expanderRect, m_window->Appearance->Background, m_window->Appearance->BoxBorderColor, true);
			
				graphics.DrawArrow(expanderRect,
					arrowLength,
					arrowWidth,
					node->isExpanded ? Graphics::ArrowDirection::Downwards : Graphics::ArrowDirection::Right,
					m_window->Appearance->Foreground2nd,
					true,
					node->isExpanded ? m_window->Appearance->Foreground2nd : m_window->Appearance->BoxBackground
				);
			}
			
			int textPaddingX = m_window->ToScale(5); 
			int textX = indentX + expanderSize + textPaddingX;
			Rectangle textRect{ textX, drawY + ((nodeHeight - (int)graphics.GetTextExtent().Height)/2), (uint32_t)(clientWidth - textX), (uint32_t)nodeHeight };
        
			Color textColor = isSelected ? appearance->HighlightTextColor : appearance->Foreground;
			graphics.DrawString(textRect, node->text, textColor);
			
			if (m_isDragging && m_dropTargetNode == node)
			{
				int indicatorX = (flatNode.Level * depthMultiplier) - scrollX + clientArea.Y;
    
				Color indicatorColor = Color(0, 120, 215, 255);

				if (m_dropPosition == DropPosition::Inside)
				{
					graphics.DrawRectangle(rowRect, indicatorColor, false);
				}
				else if (m_dropPosition == DropPosition::Before)
				{
					Rectangle lineRect{ indicatorX, drawY, clientArea.Width - indicatorX, 2 };
					graphics.DrawRectangle(lineRect, indicatorColor, true);
				}
				else if (m_dropPosition == DropPosition::After)
				{
					Rectangle lineRect{ indicatorX, drawY + nodeHeight - 2, clientArea.Width  - indicatorX, 2 };
					graphics.DrawRectangle(lineRect, indicatorColor, true);
				}
			}
		}
		
		/*
		bool enabled = true;
		auto nodeHeight = m_window->ToScale(m_appearance->TreeItemHeight);
		auto nodeTextMargin = m_window->ToScale(4u);
		auto nodeHeightInt = static_cast<int>(nodeHeight);
		auto nodeHeightHalfInt = nodeHeightInt >> 1;
		Point offset{ m_viewport.m_backgroundRect.X - m_scrollOffset.X, m_viewport.m_backgroundRect.Y - m_scrollOffset.Y };

		auto iconSize = m_window->ToScale(m_window->Appearance->SmallIconSize);
		auto iconMargin = m_window->ToScale(2);
		auto expanderSize = m_window->ToScale(m_appearance->ExpanderButtonSize);
		auto depthWidthMultiplier = m_window->ToScale(m_appearance->DepthWidthMultiplier);

		int expanderMarginX = static_cast<int>(depthWidthMultiplier - expanderSize) >> 1;
		int expanderMarginY = (nodeHeightInt - static_cast<int>(expanderSize)) >> 1;

		int i = m_viewport.m_startingVisibleIndex;
		for (auto& node : m_visibleNodes)
		{
			auto depth = CalculateNodeDepth(node);
			int depthOffsetX = static_cast<int>((depth - 1) * depthWidthMultiplier);

			Rectangle expanderRect{ offset.X + depthOffsetX + expanderMarginX, offset.Y + nodeHeightInt * i + expanderMarginY, expanderSize, expanderSize };
			Rectangle nodeRect{ offset.X + depthOffsetX + (int)depthWidthMultiplier, offset.Y + nodeHeightInt * i, m_viewport.m_contentSize.Width, nodeHeight };
			nodeRect.Width -= nodeRect.X;

			bool isLastSelected = node == m_mouseSelection.m_selectedNode;
			bool isSelected = node->isSelected;
			bool isHovered = node == m_mouseSelection.m_hoveredNode;

			if (isSelected)
			{
				auto lineColor = enabled ? (isLastSelected ? m_appearance->Foreground2nd : (isSelected ? m_appearance->BoxBorderHighlightColor : m_appearance->BoxBorderColor)) : m_appearance->BoxBorderDisabledColor;

				graphics.DrawRectangle(nodeRect, m_window->Appearance->HighlightColor, true);
				graphics.DrawRectangle(nodeRect, lineColor, false);
			}
			else if (isHovered)
			{
				graphics.DrawRectangle(nodeRect, m_window->Appearance->ItemCollectionHightlightBackground, true);
			}
			else if (isLastSelected)
			{
				graphics.DrawRectangle(nodeRect, m_window->Appearance->Foreground2nd, false);
			}

			int contentOffsetX = 0;
			if (m_drawImages)
			{
				if (node->icon)
				{
					auto positionY = (nodeHeight - iconSize) >> 1;
					node->icon.Paste(graphics, { nodeRect.X + iconMargin, nodeRect.Y + (int)positionY, iconSize , iconSize });
				}
				contentOffsetX += iconSize + iconMargin * 2;
			}

			if (node->firstChild)
			{
				int arrowWidth = m_window->ToScale(4);
				int arrowLength = m_window->ToScale(2);
				graphics.DrawRoundRectBox(expanderRect, m_window->Appearance->Background, m_window->Appearance->BoxBorderColor, true);
			
				graphics.DrawArrow(expanderRect,
					arrowLength,
					arrowWidth,
					node->isExpanded ? Graphics::ArrowDirection::Downwards : Graphics::ArrowDirection::Right,
					m_window->Appearance->Foreground2nd,
					true,
					node->isExpanded ? m_window->Appearance->Foreground2nd : m_window->Appearance->BoxBackground
				);
			}

			graphics.DrawString({ nodeRect.X + contentOffsetX + (int)nodeTextMargin, nodeRect.Y + (int)(nodeHeight - graphics.GetTextExtent().Height) / 2 }, node->text, m_window->Appearance->Foreground);
			++i;
		}*/
	}

	/*void TreeBoxReactor::Module::DrawNavigationLines(Graphics& graphics)
	{
		auto nodeHeight = m_window->ToScale(m_appearance->TreeItemHeight);
		auto nodeTextMargin = m_window->ToScale(8u);
		auto nodeHeightInt = static_cast<int>(nodeHeight);
		auto nodeHeightHalfInt = nodeHeightInt >> 1;
		Point offset{ m_viewport.m_backgroundRect.X - m_scrollOffset.X, m_viewport.m_backgroundRect.Y - m_scrollOffset.Y };

		auto iconSize = m_window->ToScale(m_window->Appearance->SmallIconSize);
		auto expanderSize = m_window->ToScale(m_appearance->ExpanderButtonSize);
		auto depthWidthMultiplier = m_window->ToScale(m_appearance->DepthWidthMultiplier);

		int expanderMarginX = static_cast<int>(depthWidthMultiplier - expanderSize) >> 1;
		int expanderMarginY = (nodeHeightInt - static_cast<int>(expanderSize)) >> 1;

		auto lineColor = m_window->Appearance->BoxBorderColor;
		Graphics::LineStyle lineStyle = Graphics::LineStyle::Dotted;
		int lineWidth = m_window->ToScale(1);

		uint32_t minDepth = (std::numeric_limits<uint32_t>::max)();
		int i = m_viewport.m_startingVisibleIndex;
		for (auto& node : m_visibleNodes)
		{
			auto depth = CalculateNodeDepth(node);
			int depthOffsetX = static_cast<int>((depth - 1) * depthWidthMultiplier);

			Rectangle expanderRect{ offset.X + depthOffsetX + expanderMarginX, offset.Y + nodeHeightInt * i + expanderMarginY, expanderSize, expanderSize };
			Rectangle nodeRect{ offset.X + depthOffsetX + (int)depthWidthMultiplier, offset.Y + nodeHeightInt * i, m_viewport.m_contentSize.Width, nodeHeight };
			nodeRect.Width -= nodeRect.X;

			auto expanderRectMidX = static_cast<int>((expanderRect.X * 2 + expanderRect.Width)) >> 1;
			Point startPointV{ expanderRectMidX, nodeRect.Y };
			Point endPointV{ startPointV.X, nodeRect.Y + nodeHeightInt };

			if (i == 0)
			{
				startPointV.Y += nodeHeightHalfInt;
			}

			if (node->prevSibling && !IsVisibleNode(node->prevSibling))
			{
				startPointV.Y = offset.Y + nodeHeightInt * m_viewport.m_startingVisibleIndex;
			}

			if (!node->nextSibling)
			{
				endPointV.Y -= nodeHeightHalfInt;
			}
			else
			{
				int nextSiblingIndex = -1;
				if (IsVisibleNode(node->nextSibling, nextSiblingIndex))
				{
					endPointV.Y = m_viewport.m_backgroundRect.Y + offset.Y + nodeHeightInt * (nextSiblingIndex + m_viewport.m_startingVisibleIndex);
				}
				else
				{
					endPointV.Y = offset.Y + nodeHeightInt * (m_viewport.m_endingVisibleIndex + 1);
				}
			}
			graphics.DrawLine(startPointV, endPointV, static_cast<float>(lineWidth), lineColor, lineStyle);
			
			Point startPointH{ expanderRectMidX + lineWidth * 2, nodeRect.Y + nodeHeightHalfInt };
			Point endPointH{ startPointH.X + (int)(depthWidthMultiplier / 2) - lineWidth * 2, startPointH.Y};
			graphics.DrawLine(startPointH, endPointH, static_cast<float>(lineWidth), lineColor, lineStyle);

			minDepth = (std::min)(minDepth, depth);
			++i;
		}

		if (minDepth > 1 && minDepth < (std::numeric_limits<uint32_t>::max)())
		{
			auto currentDepth = minDepth - 1;
			auto parentVisible = m_visibleNodes[0]->parent;
			auto depthParentVisible = CalculateNodeDepth(parentVisible);
			while (parentVisible && depthParentVisible > currentDepth)
			{
				parentVisible = parentVisible->parent;
				--depthParentVisible;
			}

			while (currentDepth > 0)
			{
				if (parentVisible && !parentVisible->nextSibling)
				{
					--currentDepth;
					parentVisible = parentVisible->parent;
					continue;
				}

				int nodeOffsetX = (currentDepth - 1) * depthWidthMultiplier;

				Point startPointV{ offset.X + nodeOffsetX + expanderMarginX + static_cast<int>(expanderSize) / 2, offset.Y + nodeHeightInt * m_viewport.m_startingVisibleIndex };
				Point endPointV{ startPointV.X, offset.Y + nodeHeightInt * (m_viewport.m_endingVisibleIndex + 1) };

				graphics.DrawLine(startPointV, endPointV, static_cast<float>(lineWidth), lineColor, lineStyle);

				--currentDepth;
				parentVisible = parentVisible->parent;
			}
		}
	}*/

	void TreeBoxReactor::Module::EnableMultiselection(bool enabled)
	{
		m_multiselection = enabled;
	}

	TreeNodeHandle TreeBoxReactor::Module::CleanKey(const TreeNodeHandle& key)
	{
		if (key.empty() || key[key.size() - 1] != '/')
		{
			return key;
		}

		return key.substr(0, key.size() - 1);
	}

	/*TreeBoxItem TreeBoxReactor::Module::Insert(const TreeNodeHandle& key, const std::string& text)
	{
		auto cleanKey = CleanKey(key);
		if (cleanKey.empty())
		{
			return {};
		}

		auto parts = StringUtils::Split(key, '/');
		TreeNodeType* current = &m_root;
		for (const auto& part : parts)
		{
			current = current->Add(part, text, current);
		}

		return { current, this };
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const TreeNodeHandle& key, const std::string& text, TreeNodeType* parentNode)
	{
		auto cleanKey = CleanKey(key);
		if (cleanKey.empty())
		{
			return {};
		}

		auto parts = StringUtils::Split(key, '/');
		TreeNodeType* current = parentNode;
		for (const auto& part : parts)
		{
			current = current->Add(part, text, current);
		}

		return { current, this };
	}*/

	/*TreeBoxItem TreeBoxReactor::Module::Find(const TreeNodeHandle& handle)
	{
		if (handle.empty())
		{
			return {};
		}

		auto parts = StringUtils::Split(handle, '/');
		auto current = &m_root;
		for (auto& part : parts)
		{
			current = current->Find(part);
			if (!current)
			{
				return {};
			}
		}

		return { current, this };
	}*/

	TreeNodeHandle TreeBoxReactor::Module::GenerateUniqueHandle(const TreeNodeHandle& key, TreeNodeType* parentNode)
	{
		if (parentNode)
		{
			return !parentNode->key.empty() && parentNode->key.back() == '/' ? 
				parentNode->key + key :
				parentNode->key + "/" + key;
		}
		return key;
	}

	void TreeBoxReactor::Module::Erase(const TreeNodeHandle& handle)
	{
		/*auto item = Find(handle);
		if (!item)
		{
			return;
		}

		EraseNode(item.m_node);

		UpdateScrollData();*/

		GUI::UpdateWindow(m_window);
	}

	void TreeBoxReactor::Module::Erase(TreeBoxItem item)
	{
		EraseNode(item.m_node);

		UpdateScrollData();

		GUI::UpdateWindow(m_window);
	}

	void TreeBoxReactor::Module::EraseNode(TreeNodeType* node)
	{
		if (node == nullptr)
			return;

		/*auto current = node->firstChild;
		while (current)
		{
			auto temp = current->nextSibling;
			EraseNode(current);

			current = temp;
		}*/
		//node->m_lookup.clear();
		
		//auto eraseResult = node->parent->m_lookup.erase(node->key);
	}

	void TreeBoxReactor::Module::UpdateScrollData()
	{
		if (!m_scrollableView)
		{
			return;
		}
		
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		auto nodeHeight = m_window->ToScale(appearance->TreeItemHeight);
		auto clientArea = m_control->GetClientArea();
			
		m_scrollableView->SetViewRect(clientArea);
		
		int m_maxWidthEstimated = m_visibleWidths.empty() ? 0 : *m_visibleWidths.rbegin();
		
		Size contentSize;
		contentSize.Width = m_maxWidthEstimated == 0 ? clientArea.Width : (uint32_t)m_maxWidthEstimated;
		contentSize.Height = static_cast<uint32_t>(m_flatVisibleTree.size()) * nodeHeight;
		m_scrollableView->SetContentSize(contentSize);
	}

	void TreeBoxReactor::Module::EmitSelectionEvent()
	{
		/*ArgTreeBoxSelection argTreeBox;
		argTreeBox.Items.resize(m_mouseSelection.m_selections.size());
		for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			argTreeBox.Items[i] = { m_mouseSelection.m_selections[i], this };
		}
		reinterpret_cast<TreeBoxEvents*>(m_window->Events.get())->Selected.Emit(argTreeBox);*/
	}

	void TreeBoxReactor::Module::EmitExpansionEvent(TreeNodeType* node)
	{
		auto item = TreeBoxItem{ node, this };
		ArgTreeBox argTreeBox(item, node->isExpanded);
		reinterpret_cast<TreeBoxEvents*>(m_window->Events.get())->Expanded.Emit(argTreeBox);
	}

	bool TreeBoxReactor::Module::Expand(TreeBoxItem item)
	{
		if (!item || item.m_node->children.empty())
		{
			return false;
		}
		bool needUpdate = !item.m_node->isExpanded;
		item.m_node->isExpanded = true;
		
		return needUpdate;
	}

	void TreeBoxReactor::Module::CollapseNode(TreeNodeType* node)
	{
		if (!node->isExpanded) return;
		node->isExpanded = false;

		auto it = std::find_if(m_flatVisibleTree.begin(), m_flatVisibleTree.end(),
							   [node](const FlatNode& fn) { return fn.Node == node; });

		if (it == m_flatVisibleTree.end()) 
		{
			return;
		}
		
		int parentLevel = it->Level;
		auto eraseStart = it + 1;
		auto eraseEnd = eraseStart;
		
		while (eraseEnd != m_flatVisibleTree.end() && eraseEnd->Level > parentLevel) 
		{
			int totalWidth = CalculateNodeWidth(eraseEnd->Node, eraseEnd->Level);
			auto it = m_visibleWidths.find(totalWidth);
			if (it != m_visibleWidths.end())
			{
				m_visibleWidths.erase(it);
			}
			
			++eraseEnd;
		}

		m_flatVisibleTree.erase(eraseStart, eraseEnd);
		m_needsRecalculate = true;
	}

	void TreeBoxReactor::Module::ExpandNode(TreeNodeType* node)
	{
		if (!node || node->isExpanded || node->children.empty()) return;
		node->isExpanded = true;

		RebuildFlatTree();
		
		m_needsRepaint = true;
		m_needsRecalculate = true;
	}

	int TreeBoxReactor::Module::CalculateNodeWidth(TreeNodeType* node, int level)
	{
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		if (node->cachedTextWidth == -1) // Solo medimos si nunca se ha medido
		{
			int textWidth = m_graphics->GetTextExtent(node->text).Width;
			//int iconWidth = m_appearance->ExpanderButtonSize + 5; // padding
			node->cachedTextWidth = textWidth /*+ iconWidth*/;
		}
		return node->cachedTextWidth + (level * (int)appearance->DepthWidthMultiplier);
	}

	void TreeBoxReactor::Module::RebuildFlatTree()
	{
		m_flatVisibleTree.clear();
		m_visibleWidths.clear();
		
		const TreeNodeType* root = m_model.GetRoot();
		for (size_t i = 0; i < root->children.size(); ++i)
		{
			bool isLast = (i == root->children.size() - 1);
			CollectVisibleNodes(root->children[i], 0, 0, isLast);
		}

		int maxWidth = m_visibleWidths.empty() ? 0 : *m_visibleWidths.rbegin();
		int rightPadding = 10; 
    
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		auto nodeHeight = static_cast<int>(m_window->ToScale(appearance->TreeItemHeight));
		int totalHeight = static_cast<int>(m_flatVisibleTree.size()) * nodeHeight;

		m_scrollableView->SetContentSize(Size{ static_cast<uint32_t>(maxWidth + rightPadding), static_cast<uint32_t>(totalHeight) });
		
		m_needsRecalculate = true;
		m_needsRepaint = true;
	}

	void TreeBoxReactor::Module::CollectVisibleNodes(TreeNodeType* node, int level, uint32_t lineMask, bool isLastChild)
	{
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		m_flatVisibleTree.emplace_back(node, level, isLastChild, lineMask);

		if (node->cachedTextWidth == -1)
		{
			Size textSize = m_graphics->GetTextExtent(node->text);
			node->cachedTextWidth = static_cast<int>(textSize.Width);
		}
		
		auto expanderSize = static_cast<int>(m_window->ToScale(appearance->ExpanderButtonSize));
		auto depthMultiplier = static_cast<int>(m_window->ToScale(appearance->DepthWidthMultiplier));
		int textPaddingX = 5;

		int rowTotalWidth = (level * depthMultiplier) + expanderSize + textPaddingX + node->cachedTextWidth;
		m_visibleWidths.insert(rowTotalWidth);

		if (node->isExpanded && !node->children.empty())
		{
			uint32_t childMask = lineMask;
			if (!isLastChild) childMask |= (1 << level);  
			else              childMask &= ~(1 << level);
			
			for (size_t i = 0; i < node->children.size(); ++i)
			{
				bool childIsLast = (i == node->children.size() - 1);
				CollectVisibleNodes(node->children[i], level + 1, childMask, childIsLast);
			}
		}
	}

	void TreeBoxReactor::Module::SetIcon(TreeNodeType* node, const Image& icon)
	{
		node->icon = icon;
		
		m_drawImages = true;

		if (true)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	void TreeBoxReactor::Module::SetText(TreeNodeType* node, const std::string& newText)
	{
		if (node->text == newText)
			return;

		node->text = newText;
		node->cachedTextWidth = -1;
		RebuildFlatTree();
		
		GUI::UpdateWindow(m_window);
	}

	void TreeBoxItem::Collapse()
	{
		/*if (m_module->Collapse(*this))
		{
			m_module->Update();
			m_module->Draw();
		}*/
	}

	void TreeBoxItem::Expand()
	{
		if (m_module->Expand(*this))
		{
			m_module->Update();
			m_module->Draw();
		}
	}

	void TreeBoxItem::Select()
	{
		/*bool needUpdate = false;
		bool emitSelectionEvent = false;
		if (m_module->m_multiselection)
		{
			if (m_module->HandleMultiSelection(m_node))
			{
				needUpdate = true;
				emitSelectionEvent = true;
			}
		}
		else
		{
			if (m_module->UpdateSingleSelection(m_node))
			{
				needUpdate = true;
				emitSelectionEvent = true;
			}
		}
		if (needUpdate)
		{
			m_module->Update();
			m_module->Draw();
		}
		if (emitSelectionEvent)
		{
			m_module->EmitSelectionEvent();
		}*/
	}

	std::any& TreeBoxItem::UserData()
	{
		return m_node->userData;
	}

	const std::any& TreeBoxItem::UserData() const
	{
		return m_node->userData;
	}

	std::vector<TreeBoxItem> TreeBoxReactor::Module::GetSelected()
	{
		std::vector<TreeBoxItem> result;
		auto selectedItems = m_selectionController.GetSelectedItems();
		result.reserve(selectedItems.size());
		
		for (auto& selectedItem : selectedItems)
		{
			result.emplace_back(selectedItem, this);
		}
		
		return result;
	}

	bool TreeBoxReactor::Module::ShowNavigationLines(bool visible)
	{
		if (m_showNavigationLines == visible)
			return false;

		m_showNavigationLines = visible;
		return true;
	}

	bool TreeBoxReactor::Module::IsDescendantOf(TreeNodeType* node, TreeNodeType* potentialAncestor) const
	{
		TreeNodeType* current = node;
		while (current)
		{
			if (current == potentialAncestor)
			{
				return true;
			}
			current = current->parent;
		}
		return false;
	}

	void TreeBoxReactor::Module::InitScrollableView()
	{
		m_scrollableView = std::make_unique<ScrollableView>(m_window);
		
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		auto nodeHeightInt = static_cast<int>(m_window->ToScale(appearance->TreeItemHeight));
		
		m_scrollableView->SetScrollStep(nodeHeightInt, 20);
		m_scrollableView->SetOnScrollChange([this]()
			{
				m_needsRepaint = true;
				GUI::MarkAsNeedUpdate(m_window);
			});
		
		m_treeRangeResolver = [this](const TreeNodeType* anchor, const TreeNodeType* current)
		{
			std::vector<TreeNodeType*> range;
        
			auto itAnchor = std::find_if(m_flatVisibleTree.begin(), m_flatVisibleTree.end(), 
										 [anchor](const FlatNode& fn) { return fn.Node == anchor; });
			auto itCurrent = std::find_if(m_flatVisibleTree.begin(), m_flatVisibleTree.end(), 
										  [current](const FlatNode& fn) { return fn.Node == current; });

			if (itAnchor == m_flatVisibleTree.end() || itCurrent == m_flatVisibleTree.end())
			{
				range.push_back(const_cast<TreeNodeType*>(current));
				return range;
			}

			auto start = std::distance(m_flatVisibleTree.begin(), itAnchor);
			auto end = std::distance(m_flatVisibleTree.begin(), itCurrent);
			if (start > end)
			{
				std::swap(start, end);
			}

			for (auto i = start; i <= end; ++i)
			{
				range.push_back(m_flatVisibleTree[i].Node);
			}
			return range;
		};
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
		auto& module = GetReactor().GetModule();
		
		auto root = module.m_model.GetRoot();
		bool needUpdate = !root->children.empty();
		module.m_selectionController.Clear();
		
		module.m_flatVisibleTree.clear();
		module.m_visibleWidths.clear();
		
		module.m_model.Clear();

		module.m_focusedNode = nullptr;
		module.m_hoveredNode = nullptr;
		
		module.m_needsRecalculate = true;
		module.m_needsRepaint = true;
		
		module.UpdateScrollData();

		if (needUpdate)
		{
			GUI::UpdateWindow(module.m_window);
		}
	}

	void TreeBox::CollapseAll()
	{
		auto& module = GetReactor().GetModule();
    
		std::function<void(TreeNodeType*)> collapseRecursive = [&](TreeNodeType* node)
		{
			if (!node)
			{
				return;
			}
			
			node->isExpanded = false;
			for (auto& childPair : node->children)
			{
				collapseRecursive(childPair);
			}
		};
		TreeNodeType* root = module.m_model.GetRoot();
		for (auto& childPair : root->children)
		{
			collapseRecursive(childPair);
		}

		module.RebuildFlatTree();
		module.Draw();
	}

	void TreeBox::CollapseAll(TreeBoxItem item)
	{
		if (!item)
		{
			return;
		}
		auto& module = GetReactor().GetModule();

		std::function<void(TreeNodeType*)> collapseRecursive = [&](TreeNodeType* node)
		{
			node->isExpanded = false;
			for (auto& childPair : node->children)
			{
				collapseRecursive(childPair);
			}
		};

		collapseRecursive(item.GetNode());
		
		module.RebuildFlatTree();
		module.Draw();
	}

	void TreeBox::Erase(const TreeNodeHandle& key)
	{
		GetReactor().GetModule().Erase(key);
	}

	void TreeBox::Erase(TreeBoxItem item)
	{
		if (!item || !item.GetNode())
		{
			return;
		}
		auto& module = GetReactor().GetModule();
		auto node = item.GetNode();
		if (module.m_selectionController.IsSelected(node))
		{
			module.m_selectionController.SetSelected(node, false); 
		}
		if (module.m_focusedNode == node)
		{
			module.m_focusedNode = nullptr;
		}
		if (module.m_hoveredNode == node)
		{
			module.m_hoveredNode = nullptr;
		}

		module.m_model.Erase(node);

		module.RebuildFlatTree();
	}

	TreeBoxItem TreeBox::Find(const TreeNodeHandle& key)
	{
		auto& module = GetReactor().GetModule();
		TreeNodeType* node = module.m_model.Find(key);
    
		return { node, &module };
	}

	TreeBoxItem TreeBox::Insert(const TreeNodeHandle& key, const std::string& text)
	{
		auto& module = GetReactor().GetModule();
		auto cleanKey = module.CleanKey(key);
		if (cleanKey.empty())
		{
			return {};
		}
		char separator = '/';
		TreeNodeType* parentNode = nullptr;
		
		size_t lastSeparatorPos = cleanKey.find_last_of(separator);
		if (lastSeparatorPos != std::string::npos)
		{
			std::string parentKey = cleanKey.substr(0, lastSeparatorPos);
        
			parentNode = module.m_model.Find(parentKey);
		}
		
		TreeNodeType* newNode = module.m_model.Insert(cleanKey, text, parentNode);
		
		module.RebuildFlatTree();
		module.UpdateScrollData();
		module.Draw();
		
		return { newNode, &module };
	}

	TreeBoxItem TreeBox::Insert(TreeBoxItem parent, const TreeNodeHandle& key, const std::string& text)
	{
		auto& module = GetReactor().GetModule();
		auto cleanKey = module.CleanKey(key);
		if (cleanKey.empty())
		{
			return {};
		}
		TreeNodeType* parentNode = parent ? parent.GetNode() : nullptr;
		
		TreeNodeType* newNode = module.m_model.Insert(cleanKey, text, parentNode);

		if (!parentNode || parentNode->isExpanded)
		{
			module.RebuildFlatTree();
			module.UpdateScrollData();
			module.Draw();
		}
		return { newNode, &module };
	}

	void TreeBox::DeselectAll()
	{
		/*auto& module = GetReactor().GetModule();
		auto& selection = module.m_mouseSelection.m_selections;
		if (selection.empty())
		{
			return;
		}

		for(auto& node : selection)
		{
			module.m_mouseSelection.Deselect(node);
		}
		module.EmitSelectionEvent();*/
	}

	void TreeBox::ExpandAll()
	{
		auto& module = GetReactor().GetModule();
		/*if (module.ExpandAll())
		{
			module.Update();
			module.Draw();
		}*/
	}

	void TreeBox::ExpandAll(TreeBoxItem item)
	{
		auto& module = GetReactor().GetModule();
		/*if (module.ExpandAll(item))
		{
			module.Update();
			module.Draw();
		}*/
	}

	std::string TreeBox::GetKeyPath(TreeBoxItem item, char separator)
	{
		if (!item) return "";
		return GetReactor().GetModule().m_model.GetKeyPath(item.GetNode(), separator);
	}

	std::vector<TreeBoxItem> TreeBox::GetSelected()
	{
		return GetReactor().GetModule().GetSelected();
	}

	void TreeBox::EnableMultiselection(bool enabled)
	{
		GetReactor().GetModule().EnableMultiselection(enabled);
	}

	void TreeBox::ShowNavigationLines(bool visible)
	{
		auto& module = GetReactor().GetModule();
		if (module.ShowNavigationLines(visible))
		{
			module.Update();
			module.Draw();
		}
	}
}
