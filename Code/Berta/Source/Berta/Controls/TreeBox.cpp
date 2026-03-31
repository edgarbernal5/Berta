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
	TreeModel::TreeModel()
	{
		m_root = m_nodePool.Allocate(L"$$ROOT$$", L"", nullptr);
		m_root->isExpanded = true;
	}

	TreeModel::~TreeModel()
	{
		Clear();
		m_nodePool.Deallocate(m_root);
	}

	TreeNodeType* TreeModel::Insert(const TreeNodeHandle& key, const std::wstring& text, TreeNodeType* parent)
	{
		std::vector<std::wstring> pathParts = StringUtils::Split(key, L'/');
		if (pathParts.empty() || !m_root)
		{
			return nullptr;
		}
		
		TreeNodeType* currentParent = parent == nullptr ? m_root : parent; 

		for (size_t i = 0; i < pathParts.size(); ++i)
		{
			const std::wstring& relativeKey = pathParts[i];
        
			TreeNodeType* existingChild = nullptr;
			for (auto* child : currentParent->children)
			{
				if (child->key == relativeKey)
				{
					existingChild = child;
					break;
				}
			}

			if (existingChild)
			{
				currentParent = existingChild;
			}
			else
			{
				TreeNodeType* newNode = m_nodePool.Allocate();
            
				newNode->key = relativeKey;
				newNode->text = text; 
				newNode->parent = currentParent;
            
				currentParent->children.emplace_back(newNode);
            
				currentParent = newNode;
			}
		}

		return currentParent;
	}

	void TreeModel::MoveNode(TreeNodeType* nodeToMove, TreeNodeType* targetNode, DropPosition pos)
	{
		if (!nodeToMove || !targetNode || nodeToMove == targetNode || nodeToMove == m_root)
		{
			return;
		}

		if (nodeToMove->parent)
		{
			auto& oldSiblings = nodeToMove->parent->children;
			oldSiblings.erase(std::remove(oldSiblings.begin(), oldSiblings.end(), nodeToMove), oldSiblings.end());
		}

		if (pos == DropPosition::Inside)
		{
			nodeToMove->parent = targetNode;
			targetNode->children.emplace_back(nodeToMove);
			targetNode->isExpanded = true;
		}
		else // Before o After
		{
			TreeNodeType* newParent = targetNode->parent;
			nodeToMove->parent = newParent;
        
			auto& newSiblings = newParent->children;
			auto itTarget = std::find(newSiblings.begin(), newSiblings.end(), targetNode);
        
			if (pos == DropPosition::Before)
			{
				newSiblings.insert(itTarget, nodeToMove);
			}
			else
			{
				newSiblings.insert(itTarget + 1, nodeToMove);
			}
		}
	}

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
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_module.m_window->Appearance.get());
		
		if (m_module.m_needsRecalculate)
		{
			auto nodeHeight = static_cast<int>(window->ToScale(appearance->TreeItemHeight));
			auto maxWidth = m_module.m_visibleWidths.empty() ? 0 : *m_module.m_visibleWidths.rbegin();
        
			m_module.m_scrollableView->SetContentSize(Size{ maxWidth, static_cast<uint32_t>(m_module.m_flatVisibleTree.size()) * nodeHeight });
			m_module.m_needsRecalculate = false;
		}
		
		auto globalRect = window->ClientSize.ToRectangle();
		graphics.FillRectangle(globalRect, window->Appearance->BoxBackground);
		if (!m_control->IsBorderless())
		{
			graphics.DrawRectangle(globalRect, appearance->BoxBorderColor);
				
			Rectangle localBorderRect = m_control->GetClientArea();
			graphics.SetClipping(localBorderRect);
		}
		
		m_module.DrawTreeNodes(graphics);

		if (m_module.m_scrollableView->HasVerticalScroll() && m_module.m_scrollableView->HasHorizontalScroll())
		{
			auto scrollSize = m_module.m_window->ToScale(m_module.m_window->Appearance->ScrollBarSize);
			graphics.FillRectangle({ (int)(m_module.m_window->ClientSize.Width - scrollSize) - 1, (int)(m_module.m_window->ClientSize.Height - scrollSize) - 1, scrollSize, scrollSize }, m_module.m_window->Appearance->Background);
		}
		
		if (!m_control->IsBorderless())
		{
			graphics.EndClipping();
		}
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
			m_module.EmitExpansionEvent(clickedNode);
			
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}
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
		int expanderStartX = static_cast<int>(flatNode.Level * depthMultiplier) - scrollX + clientArea.X + expanderMarginX;
		int expanderEndX = expanderStartX + static_cast<int>(expanderSize);

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

			m_module.EmitSelectionEvent();
			GUI::MarkAsNeedUpdate(m_module.m_window);
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
		auto clientArea = m_module.m_scrollableView->GetClientArea();
		if (m_module.m_draggedNode && !m_module.m_isDragging)
		{
			if (std::abs(args.Position.X - m_module.m_dragStartPoint.X) > dragThreshold || 
				std::abs(args.Position.Y - m_module.m_dragStartPoint.Y) > dragThreshold)
			{
				m_module.m_isDragging = true;
				
				ArgTreeDragDrop arguments{ TreeBoxItem(m_module.m_draggedNode, &m_module), {}, DropPosition::None };
				reinterpret_cast<TreeBoxEvents*>(m_module.m_window->Events.get())->DragStart.Emit(arguments);
				
				if (arguments.Cancel) 
				{
					m_module.ResetDragState();
					return;
				}
			}
		}
		auto nodeHeight = static_cast<int>(m_module.m_window->ToScale(appearance->TreeItemHeight));
		int absoluteY = args.Position.Y + m_module.m_scrollableView->GetScrollOffset().Y - clientArea.Y;
		
		size_t hoveredIndex = absoluteY / nodeHeight;
		if (m_module.m_isDragging)
		{			
			if (hoveredIndex < m_module.m_flatVisibleTree.size())
			{
				TreeNodeType* hoverNode = m_module.m_flatVisibleTree[hoveredIndex].Node;
				if (hoverNode != m_module.m_draggedNode && !m_module.IsDescendantOf(hoverNode, m_module.m_draggedNode))
				{
					m_module.m_dropTargetNode = hoverNode;
                
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
				ArgTreeDragDrop arguments{ TreeBoxItem(m_module.m_draggedNode, &m_module), {m_module.m_dropTargetNode, &m_module}, m_module.m_dropPosition };
				reinterpret_cast<TreeBoxEvents*>(m_module.m_window->Events.get())->DragOver.Emit(arguments);
				
				if (arguments.Cancel) 
				{
					m_module.m_dropPosition = DropPosition::None;
				}
			}
        
			m_module.m_needsRepaint = true;
			GUI::MarkAsNeedUpdate(m_module.m_window);
			return;
		}
		
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
				ArgTreeDragDrop arguments{ TreeBoxItem(m_module.m_draggedNode, &m_module), {m_module.m_dropTargetNode, &m_module}, m_module.m_dropPosition };
				auto events = reinterpret_cast<TreeBoxEvents*>(m_module.m_window->Events.get());
				events->BeforeDrop.Emit(arguments);
				
				if (!arguments.Cancel)
				{
					m_module.m_model.MoveNode(m_module.m_draggedNode, m_module.m_dropTargetNode, m_module.m_dropPosition);
            
					m_module.RebuildFlatTree(); 
					m_module.UpdateScrollData();
					
					events->NodeMoved.Emit(arguments);
				}
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
		
		if (args.Key == KeyboardKey::Escape && m_module.m_isDragging)
		{
			m_module.ResetDragState();
			GUI::MarkAsNeedUpdate(m_module.m_window);
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
				auto visibleNodesCount = static_cast<int>(m_module.m_scrollableView->GetClientArea().Height / nodeHeight);
				targetIndex = std::max<int>(0, currentIndex - visibleNodesCount);
			}
			break;
		case KeyboardKey::PageDown:
			{
				auto visibleNodesCount = static_cast<int>(m_module.m_scrollableView->GetClientArea().Height / nodeHeight);
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
			if (!m_module.m_ctrlPressed)
			{
				bool selectionChanged = m_module.m_selectionController.Select
				(
					m_module.m_focusedNode, 
					m_module.m_ctrlPressed, 
					m_module.m_shiftPressed, 
					m_module.m_treeRangeResolver
				);

				if (selectionChanged)
				{
					m_module.EmitSelectionEvent();
				}
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
		auto nodeHeightHalf = nodeHeight >> 1;
		auto expanderSize = m_window->ToScale(appearance->ExpanderButtonSize);
		auto depthMultiplier = static_cast<int>(m_window->ToScale(appearance->DepthWidthMultiplier));
		
		int expanderMarginX = static_cast<int>(depthMultiplier - expanderSize) >> 1;
		auto iconSize = m_window->ToScale(m_window->Appearance->SmallIconSize);
		int textPaddingX = m_window->ToScale(5); 
		auto iconPaddingX = m_window->ToScale(2);
		int scrollX = m_scrollableView->GetScrollOffset().X;
		int scrollY = m_scrollableView->GetScrollOffset().Y;
		auto clientArea = m_scrollableView->GetClientArea();
		int clientHeight = (int)clientArea.Height;
		int clientWidth = (int)clientArea.Width;
    
		size_t startIndex = std::max<size_t>(0ULL, static_cast<size_t>(scrollY / nodeHeight));
		size_t visibleCount = (clientArea.Height / nodeHeight) + 2; // +2 de margen
		size_t endIndex = std::min<size_t>(m_flatVisibleTree.size(), startIndex + visibleCount);

		for (size_t i = startIndex; i < endIndex; ++i)
		{
			const auto& flatNode = m_flatVisibleTree[i];
			const auto& node = flatNode.Node;
        
			int indentX = static_cast<int>(flatNode.Level * depthMultiplier) - scrollX + clientArea.X;
			int drawY = static_cast<int>(i * nodeHeight) - scrollY + clientArea.Y;
			
			if (drawY + nodeHeight < 0 || drawY > clientHeight)
			{
				continue;
			}
			
			int centerY = drawY + nodeHeightHalf;
			int currentX = indentX;
			
			Rectangle rowRect{ clientArea.X, drawY, clientArea.Width, static_cast<uint32_t>(nodeHeight) };
			
			bool isSelected = m_selectionController.IsSelected(node);
			bool isFocused = (node == m_focusedNode);
			bool isHovered = (node == m_hoveredNode);
			
			int reducedX = currentX + static_cast<int>(expanderSize) + textPaddingX;
			if (isSelected)
			{
				Rectangle reducedRowRect = rowRect;
				reducedRowRect.X += reducedX;
				reducedRowRect.Width -= reducedX;
				auto color = appearance->SelectionHighlightColor;
				//color.SetA(200);
				graphics.FillRectangle(reducedRowRect, color); 
			}
			else if (isHovered)
			{
				Rectangle reducedRowRect = rowRect;
				reducedRowRect.X += reducedX;
				reducedRowRect.Width -= reducedX;
				auto color = appearance->HighlightColor;
				//color.SetA(180);
				graphics.FillRectangle(reducedRowRect, color); 
			}
			
			if (m_showNavigationLines)
			{
				Color lineColor = appearance->BoxBorderColor;
				int startX = -scrollX;
	
				for (uint32_t currLevel = 0; currLevel < flatNode.Level; ++currLevel)
				{
					if (flatNode.VerticalLineMask & (1 << currLevel))
					{
						int lineX = startX + static_cast<int>(currLevel * depthMultiplier) + (static_cast<int>(expanderSize) / 2) + expanderMarginX;
						graphics.DrawLine({lineX, drawY}, {lineX, drawY + nodeHeight}, lineColor, LineStyle::Dotted);
					}
				}
	
				int currentLineX = startX + static_cast<int>(flatNode.Level * depthMultiplier) + (static_cast<int>(expanderSize) / 2) + expanderMarginX;
    
				// Línea horizontal (apunta hacia el icono/texto)
				graphics.DrawLine({currentLineX, centerY}, {currentLineX + (depthMultiplier / 2), centerY}, lineColor, LineStyle::Dotted);

				// Línea vertical superior (viene del nodo de arriba)
				graphics.DrawLine({currentLineX, drawY}, {currentLineX, centerY}, lineColor, LineStyle::Dotted);

				// Línea vertical inferior (continúa hacia abajo solo si NO es el último hijo)
				if (!flatNode.IsLastChild)
				{
					graphics.DrawLine({currentLineX, centerY}, {currentLineX, drawY + nodeHeight}, lineColor, LineStyle::Dotted);
				}
			}
			Rectangle expanderRect{ indentX + expanderMarginX, drawY + (nodeHeight - static_cast<int>(expanderSize)) / 2, expanderSize, expanderSize };
			
			currentX += static_cast<int>(expanderSize) + textPaddingX;
			if (m_drawImages)
			{
				if (node->icon)
				{
					Rectangle iconRect{ rowRect.X + currentX + iconPaddingX, centerY - (int)(iconSize >> 1), iconSize, iconSize };
					node->icon.Paste(graphics, iconRect);
				}
				currentX += static_cast<int>(iconSize) + iconPaddingX * 2;
			}
			
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
			
			if (isFocused)
			{
				Rectangle reducedRowRect = rowRect;
				reducedRowRect.X += reducedX;
				reducedRowRect.Width -= reducedX;
				graphics.DrawRectangle(reducedRowRect, appearance->Foreground);
			}
			
			Rectangle textRect{ currentX, drawY + ((nodeHeight - static_cast<int>(graphics.GetTextExtent().Height))/2), (uint32_t)(clientWidth - currentX), (uint32_t)nodeHeight };
        
			Color textColor = isSelected ? appearance->HighlightTextColor : appearance->Foreground;
			graphics.DrawString(textRect.Position(), node->text, textColor);
			
			if (m_isDragging && m_dropTargetNode == node)
			{
				int indicatorX = static_cast<int>(flatNode.Level * depthMultiplier) - scrollX + clientArea.Y;
    
				Color indicatorColor = Color(0, 120, 215, 255);

				if (m_dropPosition == DropPosition::Inside)
				{
					graphics.DrawRectangle(rowRect, indicatorColor);
				}
				else if (m_dropPosition == DropPosition::Before)
				{
					Rectangle lineRect{ indicatorX, drawY - 1, clientArea.Width - indicatorX, 2 };
					graphics.FillRectangle(lineRect, indicatorColor);
				}
				else if (m_dropPosition == DropPosition::After)
				{
					Rectangle lineRect{ indicatorX, drawY + nodeHeight - 1, clientArea.Width  - indicatorX, 2 };
					graphics.FillRectangle(lineRect, indicatorColor);
				}
			}
		}
	}

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

	TreeNodeHandle TreeBoxReactor::Module::GenerateUniqueHandle(const TreeNodeHandle& key, TreeNodeType* parentNode)
	{
		if (parentNode)
		{
			return !parentNode->key.empty() && parentNode->key.back() == L'/' ? 
				parentNode->key + key :
				parentNode->key + L'/' + key;
		}
		return key;
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
		auto selectedNodes = m_selectionController.GetSelectedItems();
		
		ArgTreeBoxSelection argTreeBox;
		argTreeBox.Items.reserve(selectedNodes.size());
		for (auto* node : selectedNodes)
		{
			argTreeBox.Items.emplace_back(node, this); 
		}
		reinterpret_cast<TreeBoxEvents*>(m_window->Events.get())->Selected.Emit(argTreeBox);
	}

	void TreeBoxReactor::Module::EmitExpansionEvent(TreeNodeType* node)
	{
		auto item = TreeBoxItem{ node, this };
		ArgTreeBox argTreeBox(item, node->isExpanded);
		reinterpret_cast<TreeBoxEvents*>(m_window->Events.get())->Expanded.Emit(argTreeBox);
	}

	void TreeBoxReactor::Module::CollapseNode(TreeNodeType* node)
	{
		if (!node->isExpanded)
		{
			return;
		}
		node->isExpanded = false;

		auto it = std::find_if(m_flatVisibleTree.begin(), m_flatVisibleTree.end(),
							   [node](const FlatNode& fn) { return fn.Node == node; });

		if (it == m_flatVisibleTree.end()) 
		{
			return;
		}
		
		auto parentLevel = it->Level;
		auto eraseStart = it + 1;
		auto eraseEnd = eraseStart;
		
		while (eraseEnd != m_flatVisibleTree.end() && eraseEnd->Level > parentLevel) 
		{
			auto totalWidth = CalculateNodeWidth(eraseEnd->Node, eraseEnd->Level);
			auto widthIt = m_visibleWidths.find(totalWidth);
			if (widthIt != m_visibleWidths.end())
			{
				m_visibleWidths.erase(widthIt);
			}
			
			++eraseEnd;
		}

		m_flatVisibleTree.erase(eraseStart, eraseEnd);
		m_needsRecalculate = true;
	}

	void TreeBoxReactor::Module::ExpandNode(TreeNodeType* node)
	{
		if (!node || node->isExpanded || node->children.empty())
		{
			return;
		}
		node->isExpanded = true;

		RebuildFlatTree();
		
		m_needsRepaint = true;
		m_needsRecalculate = true;
	}

	uint32_t TreeBoxReactor::Module::CalculateNodeWidth(TreeNodeType* node, uint32_t level)
	{
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		
		auto expanderSize = static_cast<int>(m_window->ToScale(appearance->ExpanderButtonSize));
		auto depthMultiplier = static_cast<int>(m_window->ToScale(appearance->DepthWidthMultiplier));
		auto iconSize = static_cast<int>(m_window->ToScale(appearance->SmallIconSize));
		int textPaddingX = m_window->ToScale(5);
		
		if (!node->cachedTextWidth.has_value())
		{
			node->cachedTextWidth = m_graphics->GetTextExtent(node->text).Width;
		}
		
		auto rowTotalWidth = *node->cachedTextWidth + (level * depthMultiplier) + expanderSize + textPaddingX;
		if (m_drawImages)
		{
			rowTotalWidth += iconSize;
		}
		return rowTotalWidth;
	}

	void TreeBoxReactor::Module::ResetDragState()
	{
		m_isDragging = false;
		m_draggedNode = nullptr;
		m_dropTargetNode = nullptr;
		m_dropPosition = DropPosition::None;
    
		m_needsRepaint = true;
		GUI::MarkAsNeedUpdate(m_window);
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

	void TreeBoxReactor::Module::CollectVisibleNodes(TreeNodeType* node, uint32_t level, uint32_t lineMask, bool isLastChild)
	{
		m_flatVisibleTree.emplace_back(node, level, isLastChild, lineMask);

		auto rowTotalWidth = CalculateNodeWidth(node, level);
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
	
	void TreeBoxItem::Collapse()
	{
		if (!m_node)
		{
			return;
		}
		m_module->CollapseNode(m_node);
		m_module->Draw();
	}

	void TreeBoxItem::Expand()
	{
		if (!m_node)
		{
			return;
		}
		m_module->ExpandNode(m_node);
		m_module->Draw();
	}

	void TreeBoxItem::Select()
	{
		bool changed = m_module->m_selectionController.Select(m_node, false, false, m_module->m_treeRangeResolver);
		if (changed)
		{
			m_module->m_focusedNode = m_node;
			
			m_module->EmitSelectionEvent();
			m_module->Draw();
		}
	}

	std::any& TreeBoxItem::UserData()
	{
		return m_node->userData;
	}

	const std::any& TreeBoxItem::UserData() const
	{
		return m_node->userData;
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
				range.emplace_back(const_cast<TreeNodeType*>(current));
				return range;
			}

			auto start = std::distance(m_flatVisibleTree.begin(), itAnchor);
			auto end = std::distance(m_flatVisibleTree.begin(), itCurrent);
			if (start > end)
			{
				std::swap(start, end);
			}

			range.reserve(end - start + 1);
			for (auto i = start; i <= end; ++i)
			{
				range.emplace_back(m_flatVisibleTree[i].Node);
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

	void TreeBox::DeselectAll()
	{
		auto& module = GetReactor().GetModule();
		module.m_selectionController.Clear();
		
		module.EmitSelectionEvent();
		module.Draw();
	}

	TreeBoxItem TreeBox::Find(const TreeNodeHandle& key)
	{
		auto& module = GetReactor().GetModule();
		TreeNodeType* node = module.m_model.Find(key);
    
		if (!node)
		{
			return {};
		}
		return { node, &module };
	}

	TreeBoxItem TreeBox::Insert(const TreeNodeHandle& absoluteKey, const std::wstring& text)
	{
		auto& module = GetReactor().GetModule();
		auto cleanAbsoluteKey = module.CleanKey(absoluteKey);
		TreeNodeType* insertedNode = module.m_model.Insert(cleanAbsoluteKey, text);
		if (insertedNode)
		{
			module.RebuildFlatTree();
			module.UpdateScrollData();
			module.Draw();
		}
		
		return { insertedNode, &module };
	}

	TreeBoxItem TreeBox::Insert(TreeBoxItem parent, const TreeNodeHandle& key, const std::wstring& text)
	{
		auto& module = GetReactor().GetModule();
		auto cleanAbsoluteKey = module.CleanKey(key);
		
		TreeNodeType* insertedNode = module.m_model.Insert(cleanAbsoluteKey, text, parent.GetNode());
		if (insertedNode)
		{
			module.RebuildFlatTree();
			module.UpdateScrollData();
			module.Draw();
		}
		
		return { insertedNode, &module };
	}
	
	void TreeBox::Erase(const TreeNodeHandle& key)
	{
		auto& module = GetReactor().GetModule();
		auto node = module.m_model.Find(key);
		if (!node)
		{
			return;
		}
		
		Erase({node, &module });
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
		module.Draw();
	}

	void TreeBox::ExpandAll()
	{
		auto& module = GetReactor().GetModule();
    
		std::function<void(TreeNodeType*)> collapseRecursive = [&](TreeNodeType* node)
		{
			if (!node)
			{
				return;
			}
			
			node->isExpanded = true;
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

	void TreeBox::ExpandAll(TreeBoxItem item)
	{
		if (!item)
		{
			return;
		}
		auto& module = GetReactor().GetModule();

		std::function<void(TreeNodeType*)> collapseRecursive = [&](TreeNodeType* node)
		{
			node->isExpanded = true;
			for (auto& childPair : node->children)
			{
				collapseRecursive(childPair);
			}
		};

		collapseRecursive(item.GetNode());
		
		module.RebuildFlatTree();
		module.Draw();
	}

	std::wstring TreeBox::GetKeyPath(TreeBoxItem item, wchar_t separator)
	{
		if (!item)
		{
			return L"";
		}
		
		return GetReactor().GetModule().m_model.GetKeyPath(item.GetNode(), separator);
	}

	std::vector<TreeBoxItem> TreeBox::GetSelected()
	{
		auto& module = GetReactor().GetModule();
		std::vector<TreeBoxItem> result;
    
		auto selectedNodes = module.m_selectionController.GetSelectedItems();
    
		result.reserve(selectedNodes.size());
		for (auto* node : selectedNodes)
		{
			result.emplace_back(node, &module); 
		}
    
		return result;
	}

	void TreeBox::Filter(const std::wstring& text)
	{
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
