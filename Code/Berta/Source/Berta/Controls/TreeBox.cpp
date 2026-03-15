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
	void TreeBoxReactor::Init(ControlBase& control, Graphics* graphics)
	{
		m_control = &control;
		m_module.m_window = control.Handle();
		m_module.m_control = m_control;
		m_module.m_graphics = graphics;

		m_module.Init();
		m_module.InitScrollableView();
	}
	
	
	TreeNodeType* TreeNodeType::Add(const TreeNodeHandle& childKey, const std::string& text_, TreeNodeType* parent_)
	{
		auto it = m_lookup.find(childKey);
		if (it != m_lookup.end())
		{
			return it->second.get();
		}

		auto child = std::make_unique<TreeNodeType>(childKey, text_, parent_);
		TreeNodeType* childPtr = child.get();
		m_lookup[childKey] = std::move(child);

		if (firstChild)
		{
			auto lastNode = firstChild;
			while (lastNode->nextSibling != nullptr)
			{
				lastNode = lastNode->nextSibling;
			}
			lastNode->nextSibling = childPtr;
			childPtr->prevSibling = lastNode;
		}
		else
		{
			firstChild = childPtr;
		}

		return childPtr;
	}

	TreeNodeType* TreeNodeType::Find(const TreeNodeHandle& key)
	{
		auto it = m_lookup.find(key);
		return it != m_lookup.end() ? it->second.get() : nullptr;
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
		
		if (m_module.m_showNavigationLines)
		{
			m_module.DrawNavigationLines(graphics);
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
		if (m_module.m_flatVisibleTree.empty()) return;

		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_module.m_window->Appearance.get());
		auto nodeHeight = static_cast<int>(m_module.m_window->ToScale(appearance->TreeItemHeight));
		int scrollY = m_module.m_scrollableView->GetScrollOffset().Y;
    
		size_t clickedIndex = (args.Position.Y + scrollY) / nodeHeight;
		if (clickedIndex >= m_module.m_flatVisibleTree.size()) 
			return;

		TreeNodeType* clickedNode = m_module.m_flatVisibleTree[clickedIndex].Node;

		if (clickedNode->firstChild)
		{
			clickedNode->isExpanded = !clickedNode->isExpanded;
			m_module.RebuildFlatTree();
		}
		/*auto nodeHeight = m_module.m_window->ToScale(appearance->TreeItemHeight);
		auto nodeHeightInt = static_cast<int>(nodeHeight);

		auto scrollOffset = m_module.m_scrollableView->GetScrollOffset();
		auto positionY = args.Position.Y - m_module.m_viewport.m_backgroundRect.Y + scrollOffset.Y;
		int index = positionY / nodeHeightInt;
		index -= m_module.m_viewport.m_startingVisibleIndex;

		auto visibleNode = m_module.m_visibleNodes[index];
		if (!visibleNode->firstChild)
			return;
		
		visibleNode->isExpanded = !visibleNode->isExpanded;
		m_module.CalculateViewport(m_module.m_viewport);

		m_module.UpdateScrollData();
		m_module.CalculateVisibleNodes();
		
		m_module.EmitExpansionEvent(visibleNode);*/
		GUI::MarkAsNeedUpdate(m_module.m_window);
	}

	void TreeBoxReactor::MouseLeave(Graphics& graphics, const ArgMouse& args)
	{
		/*bool needUpdate = m_module.m_mouseSelection.m_hoveredNode != nullptr;
		m_module.m_mouseSelection.m_hoveredNode = nullptr;

		m_module.m_hoveredArea = InteractionArea::None;
		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}*/
	}

	void TreeBoxReactor::MouseDown(Graphics& graphics, const ArgMouse& args)
	{
		if (m_module.m_flatVisibleTree.empty())
		{
			return;
		}
		
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_module.m_window->Appearance.get());
	    auto nodeHeight = static_cast<int>(m_module.m_window->ToScale(appearance->TreeItemHeight));
	    int scrollY = m_module.m_scrollableView->GetScrollOffset().Y;
	    int scrollX = m_module.m_scrollableView->GetScrollOffset().X;

	    int absoluteY = args.Position.Y + scrollY;
	    size_t clickedIndex = absoluteY / nodeHeight;

	    if (clickedIndex >= m_module.m_flatVisibleTree.size())
	    {
	    	return;
	    }

	    const auto& flatNode = m_module.m_flatVisibleTree[clickedIndex];
	    TreeNodeType* clickedNode = flatNode.Node;

	    int expanderStartX = (flatNode.Level * appearance->DepthWidthMultiplier) - scrollX;
	    int expanderEndX = expanderStartX + (int)appearance->ExpanderButtonSize;

	    if (clickedNode->firstChild && args.Position.X >= expanderStartX && args.Position.X <= expanderEndX)
	    {
	        clickedNode->isExpanded = !clickedNode->isExpanded;
	        
	        // Disparar evento de la librería
	        // ArgTreeBox eventArgs{TreeBoxItem(clickedNode), clickedNode->isExpanded};
	        // m_control->Events.Expanded(eventArgs);

	        m_module.RebuildFlatTree(); // Reconstruir caché para reflejar expansión/colapso
	        return;
	    }

	    // 3. Si no fue en el icono, usamos tu SelectionController
	    bool selectionChanged = m_module.m_selectionController.Select
		(
	        clickedNode,
	        m_module.m_ctrlPressed,
	        m_module.m_shiftPressed,
	        m_module.m_treeRangeResolver
	    );

	    if (selectionChanged)
	    {
	        m_module.m_focusedNode = clickedNode; // Actualizar el foco del teclado
	        m_module.m_needsRepaint = true;

	        // Disparar evento de selección de la librería
	        // ArgTreeBoxSelection eventArgs;
	        // eventArgs.Items = Convertir m_selectionController.GetSelectedItems() a TreeBoxItem
	        // m_control->Events.Selected(eventArgs);
	    }
		
		/*m_module.m_pressedArea = m_module.m_hoveredArea;
		bool needUpdate = false;
		bool emitSelectionEvent = false;

		if (args.ButtonState.LeftButton)
		{
			if (m_module.m_hoveredArea == InteractionArea::Expander)
			{
				auto nodeHeight = m_module.m_window->ToScale(m_module.m_appearance->TreeItemHeight);
				auto nodeHeightInt = static_cast<int>(nodeHeight);

				auto scrollOffset = m_module.m_scrollableView->GetScrollOffset();
				auto positionY = args.Position.Y - m_module.m_viewport.m_backgroundRect.Y + scrollOffset.Y;
				int index = positionY / nodeHeightInt;
				index -= m_module.m_viewport.m_startingVisibleIndex;

				m_module.m_visibleNodes[index]->isExpanded = !m_module.m_visibleNodes[index]->isExpanded;
				m_module.CalculateViewport(m_module.m_viewport);

				m_module.UpdateScrollData();
				m_module.CalculateVisibleNodes();

				needUpdate = true;
				m_module.EmitExpansionEvent(m_module.m_visibleNodes[index]);
			}
		}

		if (m_module.m_pressedArea == InteractionArea::Node)
		{
			if (m_module.m_mouseSelection.m_hoveredNode)
			{
				if (m_module.m_multiselection)
				{
					if (m_module.HandleMultiSelection(m_module.m_mouseSelection.m_hoveredNode))
					{
						needUpdate = true;
						emitSelectionEvent = true;
					}
				}
				else
				{
					if (m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_hoveredNode))
					{
						needUpdate = true;
						emitSelectionEvent = true;
					}
				}
			}
		}

		if (needUpdate && m_module.m_hoveredArea == InteractionArea::Node && m_module.m_viewport.m_needVerticalScroll)
		{
			auto nodeHeightInt = static_cast<int>(m_module.m_window->ToScale(m_module.m_appearance->TreeItemHeight));
			auto selectedIndex = m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_hoveredNode);
			auto positionY = selectedIndex * nodeHeightInt - m_module.m_scrollOffset.Y;
			auto newValue = m_module.m_scrollOffset.Y;
			if (positionY < 0)
			{
				newValue = positionY + m_module.m_scrollOffset.Y;
			}
			else if (positionY + nodeHeightInt > static_cast<int>(m_module.m_viewport.m_backgroundRect.Height))
			{
				newValue = positionY + m_module.m_scrollOffset.Y - static_cast<int>(m_module.m_viewport.m_backgroundRect.Height) + nodeHeightInt;
			}
			if (m_module.m_scrollOffset.Y != newValue)
			{
				m_module.m_scrollOffset.Y = newValue;
				m_module.m_scrollBarVert->SetValue(newValue);
			}
		}

		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}

		if (emitSelectionEvent)
		{
			m_module.EmitSelectionEvent();
		}*/
	}

	void TreeBoxReactor::MouseMove(Graphics& graphics, const ArgMouse& args)
	{
		/*auto hoveredArea = m_module.DetermineHoverArea(args.Position);
		bool needUpdate = false;
		
		if (hoveredArea == InteractionArea::Node || hoveredArea == InteractionArea::Expander)
		{
			auto nodeHeight = m_module.m_window->ToScale(m_module.m_appearance->TreeItemHeight);
			auto nodeHeightInt = static_cast<int>(nodeHeight);

			auto positionY = args.Position.Y - m_module.m_viewport.m_backgroundRect.Y + m_module.m_scrollOffset.Y;
			int index = positionY / nodeHeightInt;
			index -= m_module.m_viewport.m_startingVisibleIndex;

			needUpdate = m_module.m_visibleNodes[index] != m_module.m_mouseSelection.m_hoveredNode;
			m_module.m_mouseSelection.m_hoveredNode = m_module.m_visibleNodes[index];
		}
		else if (hoveredArea == InteractionArea::Blank)
		{
			needUpdate = m_module.m_mouseSelection.m_hoveredNode != nullptr;
			m_module.m_mouseSelection.m_hoveredNode = nullptr;
		}

		m_module.m_hoveredArea = hoveredArea;

		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}*/
	}

	void TreeBoxReactor::MouseUp(Graphics& graphics, const ArgMouse& args)
	{
	}

	void TreeBoxReactor::MouseWheel(Graphics& graphics, const ArgWheel& args)
	{
		m_module.m_scrollableView->HandleMouseWheel(args);
	}

	void TreeBoxReactor::KeyPressed(Graphics& graphics, const ArgKeyboard& args)
	{
		m_module.m_shiftPressed = m_module.m_shiftPressed || args.Key == KeyboardKey::Shift;
		m_module.m_ctrlPressed = m_module.m_ctrlPressed || args.Key == KeyboardKey::Control;

		/*bool needUpdate = false;
		bool recalculateVisibleNodes = false;
		bool emitSelectionEvent = false;
		bool emitCollapsedEvent = false;
		auto nodeHeightInt = static_cast<int>(m_module.m_window->ToScale(m_module.m_appearance->TreeItemHeight));

		if (args.Key == KeyboardKey::ArrowLeft && m_module.m_mouseSelection.m_selectedNode)
		{
			if (m_module.m_mouseSelection.m_selectedNode->firstChild && m_module.m_mouseSelection.m_selectedNode->isExpanded)
			{
				m_module.m_mouseSelection.m_selectedNode->isExpanded = false;

				recalculateVisibleNodes = true;
				needUpdate = true;
				emitCollapsedEvent = true;
			}
			else if (m_module.m_mouseSelection.m_selectedNode->parent != &m_module.m_root)
			{
				if (m_module.m_multiselection)
				{
					m_module.ClearSelection();
					if (m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_selectedNode->parent))
					{
						needUpdate = true;
						emitSelectionEvent = true;
					}
					recalculateVisibleNodes = true;
				}
				else if (!m_module.m_multiselection && m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_selectedNode->parent))
				{
					needUpdate = true;
					emitSelectionEvent = true;
				}
				recalculateVisibleNodes = true;
			}
		}
		else if (args.Key == KeyboardKey::ArrowRight && m_module.m_mouseSelection.m_selectedNode)
		{
			if (!m_module.m_mouseSelection.m_selectedNode->isExpanded)
			{
				m_module.m_mouseSelection.m_selectedNode->isExpanded = true;

				recalculateVisibleNodes = true;
				needUpdate = true;
				emitCollapsedEvent = true;
			}
			else if (m_module.m_mouseSelection.m_selectedNode->firstChild)
			{
				if (m_module.UpdateSingleSelection(m_module.m_mouseSelection.m_selectedNode->firstChild))
				{
					needUpdate = true;
					emitSelectionEvent = true;
					recalculateVisibleNodes = true;
				}
			}
		}
		else if (args.Key == KeyboardKey::Home || args.Key == KeyboardKey::End)
		{
			int newIndex = args.Key == KeyboardKey::Home ? 0 : ((int)m_module.m_viewport.m_treeSize - 1);

			auto newNode = m_module.LocateNodeIndexInTree(newIndex);
			if (newNode && m_module.UpdateSingleSelection(newNode))
			{
				m_module.m_mouseSelection.m_selectedNode = newNode;
				needUpdate = true;
				emitSelectionEvent = true;
				recalculateVisibleNodes = true;
			}
		}
		else if (args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::ArrowDown ||
			args.Key == KeyboardKey::PageUp || args.Key == KeyboardKey::PageDown)
		{
			int pageAmount = static_cast<int>(m_module.m_viewport.m_backgroundRect.Height / nodeHeightInt);
			int direction = (args.Key == KeyboardKey::ArrowUp || args.Key == KeyboardKey::PageUp) ? -1 : 1;
			int amount = direction * ((args.Key == KeyboardKey::PageDown || args.Key == KeyboardKey::PageUp) ? pageAmount : 1);
			
			auto selectedIndex = (m_module.m_mouseSelection.m_selectedNode == nullptr? (direction == -1 ? (int)m_module.m_viewport.m_treeSize : -1) : m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_selectedNode));
			auto newItemIndex = selectedIndex + amount;
			if (newItemIndex >= 0 && newItemIndex < static_cast<int>(m_module.m_viewport.m_treeSize))
			{
				if (!m_module.m_ctrlPressed)
				{
					m_module.ClearSelection();
				}

				if (m_module.m_multiselection && m_module.m_shiftPressed && m_module.m_mouseSelection.m_pivotNode != nullptr)
				{
					auto startIndex = m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_pivotNode);
					auto endIndex = newItemIndex;
					int minIndex = (std::min)(startIndex, endIndex);
					int maxIndex = (std::max)(startIndex, endIndex);

					std::vector<TreeNodeType*> rangeNodes;
					m_module.GetNodesInBetween(minIndex, maxIndex, rangeNodes);
					for (auto& node : rangeNodes)
					{
						if (!node->isSelected)
						{
							node->isSelected = true;
							m_module.m_mouseSelection.m_selections.push_back(node);
						}
					}
					needUpdate = true;
					emitSelectionEvent = true;
					m_module.m_mouseSelection.m_selectedNode = endIndex == maxIndex ? rangeNodes.back() : rangeNodes.front();
				}
				else if (m_module.m_ctrlPressed)
				{
					auto newSelectedNode = m_module.LocateNodeIndexInTree(newItemIndex);
					needUpdate = emitSelectionEvent = m_module.m_mouseSelection.m_selectedNode != newSelectedNode;
					m_module.m_mouseSelection.m_selectedNode = newSelectedNode;
				}
				else
				{
					auto newSelectedNode = m_module.LocateNodeIndexInTree(newItemIndex);
					needUpdate = emitSelectionEvent = m_module.m_mouseSelection.m_selectedNode != newSelectedNode;
					newSelectedNode->isSelected = true;

					m_module.m_mouseSelection.m_selections.push_back(newSelectedNode);
					m_module.m_mouseSelection.m_selectedNode = newSelectedNode;
					m_module.m_mouseSelection.m_pivotNode = newSelectedNode;
				}
			}
			
			
		}
		else if (args.Key == KeyboardKey::Space && m_module.m_ctrlPressed)
		{
			if (m_module.m_mouseSelection.m_selectedNode)
			{
				if (!m_module.m_multiselection && !m_module.m_mouseSelection.m_selections.empty())
				{
					m_module.m_mouseSelection.m_selections[0]->isSelected = false;
					m_module.m_mouseSelection.m_selections.clear();
				}

				auto& isSelected = m_module.m_mouseSelection.m_selectedNode->isSelected;
				isSelected = !isSelected;
				if (isSelected)
				{
					m_module.m_mouseSelection.Select(m_module.m_mouseSelection.m_selectedNode);
				}
				else
				{
					m_module.m_mouseSelection.Deselect(m_module.m_mouseSelection.m_selectedNode);
				}

				if (m_module.m_multiselection)
				{
					m_module.m_mouseSelection.m_pivotNode = m_module.m_mouseSelection.m_selectedNode;
				}
				needUpdate = true;
			}
		}

		if (emitSelectionEvent && m_module.m_viewport.m_needVerticalScroll)
		{
			auto selectedIndex = m_module.LocateNodeIndexInTree(m_module.m_mouseSelection.m_selectedNode);
			auto positionY = selectedIndex * nodeHeightInt - m_module.m_scrollOffset.Y;
			auto newValue = m_module.m_scrollOffset.Y;
			if (positionY < 0)
			{
				newValue = positionY + m_module.m_scrollOffset.Y;
				needUpdate = true;
			}
			else if (positionY + nodeHeightInt > static_cast<int>(m_module.m_viewport.m_backgroundRect.Height))
			{
				newValue = positionY + m_module.m_scrollOffset.Y - static_cast<int>(m_module.m_viewport.m_backgroundRect.Height) + nodeHeightInt;
				needUpdate = true;
			}
			if (m_module.m_scrollOffset.Y != newValue)
			{
				m_module.m_scrollOffset.Y = newValue;
				m_module.m_scrollBarVert->SetValue(newValue);
			}
		}

		if (recalculateVisibleNodes)
		{
			m_module.CalculateViewport(m_module.m_viewport);
			m_module.CalculateVisibleNodes();
		}
		m_module.UpdateScrollData();

		if (emitSelectionEvent)
		{
			m_module.EmitSelectionEvent();
		}

		if (emitCollapsedEvent)
		{
			m_module.EmitExpansionEvent(m_module.m_mouseSelection.m_selectedNode);
		}

		if (needUpdate)
		{
			GUI::MarkAsNeedUpdate(m_module.m_window);
		}*/
	}

	void TreeBoxReactor::KeyReleased(Graphics& graphics, const ArgKeyboard& args)
	{
		if (args.Key == KeyboardKey::Shift) m_module.m_shiftPressed = false;
		if (args.Key == KeyboardKey::Control) m_module.m_ctrlPressed = false;
	}


	void TreeBoxReactor::Module::Clear()
	{
		bool needUpdate = m_root.firstChild != nullptr;
		m_root.firstChild = nullptr;
		m_visibleNodes.clear();
		m_root.m_lookup.clear();

		UpdateScrollData();

		if (needUpdate)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	/*TreeBoxReactor::InteractionArea TreeBoxReactor::Module::DetermineHoverArea(const Point& mousePosition)
	{
		if (!m_window->ClientSize.IsInside(mousePosition))
		{
			return InteractionArea::None;
		}

		auto nodeHeight = m_window->ToScale(m_appearance->TreeItemHeight);
		auto nodeHeightInt = static_cast<int>(nodeHeight);

		auto positionY = mousePosition.Y - m_viewport.m_backgroundRect.Y + m_scrollOffset.Y;
		int index = positionY / nodeHeightInt;

		index -= m_viewport.m_startingVisibleIndex;
		if (index >= m_visibleNodes.size())
		{
			return InteractionArea::Blank;
		}

		auto expanderSize = m_window->ToScale(m_appearance->ExpanderButtonSize);
		auto depthWidthMultiplier = m_window->ToScale(m_appearance->DepthWidthMultiplier);
		auto nodeDepth = CalculateNodeDepth(m_visibleNodes[index]);
		int nodeOffset = (nodeDepth - 1) * depthWidthMultiplier;
		int expanderMarginX = static_cast<int>(depthWidthMultiplier - expanderSize) >> 1;
		
		Rectangle expanderRect
		{ 
			m_viewport.m_backgroundRect.X + m_scrollOffset.X + nodeOffset + expanderMarginX,
			(index + m_viewport.m_startingVisibleIndex) * nodeHeightInt + (int)(nodeHeightInt - expanderSize) / 2,
			expanderSize, expanderSize
		};
		
		Point absolutePosition = mousePosition;
		absolutePosition.Y += m_scrollOffset.Y;
		absolutePosition.X += m_scrollOffset.X;
		if (m_visibleNodes[index]->firstChild && expanderRect.IsInside(absolutePosition))
		{
			return InteractionArea::Expander;
		}

		if (absolutePosition.X >= expanderRect.X + static_cast<int>(expanderRect.Width))
		{
			return InteractionArea::Node;
		}

		return InteractionArea::Blank;
	}*/

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
		if (m_flatVisibleTree.empty()) return;
		
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		
		auto nodeHeight = static_cast<int>(m_window->ToScale(appearance->TreeItemHeight));
		auto expanderSize = m_window->ToScale(appearance->ExpanderButtonSize);
		auto depthMultiplier = static_cast<int>(m_window->ToScale(appearance->DepthWidthMultiplier));
		
		int scrollY = m_scrollableView->GetScrollOffset().Y;
		int scrollX = m_scrollableView->GetScrollOffset().X;
		auto clientArea = m_scrollableView->GetClientArea();
		int clientWidth = (int)clientArea.Width;
    
		// 1. Calcular rango estrictamente visible (O(1))
		size_t startIndex = std::max<size_t>(size_t(0), static_cast<size_t>(scrollY / nodeHeight));
		size_t visibleCount = (clientArea.Height / nodeHeight) + 2; // +2 de margen
		size_t endIndex = std::min<size_t>(m_flatVisibleTree.size(), startIndex + visibleCount);

		// 2. Dibujar solo lo que se ve
		for (size_t i = startIndex; i < endIndex; ++i)
		{
			const auto& flatNode = m_flatVisibleTree[i];
			TreeNodeType* node = flatNode.Node;
        
			// Coordenadas finales
			int drawY = static_cast<int>(i * nodeHeight) - scrollY;
			int indentX = (flatNode.Level * appearance->DepthWidthMultiplier) - scrollX;

			Rectangle rowRect{ 0, drawY, clientArea.Width, (uint32_t)nodeHeight };
			
			// Comprobar estado usando el controlador
			bool isSelected = m_selectionController.IsSelected(node);
			bool isFocused = (node == m_focusedNode);

			// --- AQUÍ VA TU LÓGICA GRÁFICA ---
			// Dibujar fondo si isSelected
			// Dibujar el icono (expandido/colapsado) si node->HasChildren()
			// Dibujar texto en indentX
			// Dibujar línea punteada si isFocused
			
			if (isSelected)
			{
				// Asume que tu Graphics tiene un método para rellenar (el 'true' en tu DrawRectangle original)
				graphics.DrawRectangle(rowRect, appearance->HighlightColor, true); 
			}
			Rectangle expanderRect{ indentX, drawY + (nodeHeight - (int)expanderSize) / 2, expanderSize, expanderSize };
			
			if (node->firstChild) // O node->HasChildren() según tu implementación
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
				
				/*if (node->isExpanded) {
					// graphics.DrawImage(m_appearance->ExpandedIcon, expanderRect);
					// O dibujado manual del icono abierto...
				} else {
					// graphics.DrawImage(m_appearance->CollapsedIcon, expanderRect);
					// O dibujado manual del icono cerrado...
				}*/
			}
			
			int textPaddingX = 5; 
			int textX = indentX + expanderSize + textPaddingX;
			Rectangle textRect{ textX, drawY, (uint32_t)(clientWidth - textX), (uint32_t)nodeHeight };
        
			// Cambiar color si está seleccionado
			Color textColor = isSelected ? appearance->HighlightTextColor : appearance->Foreground;
        
			// Asume la firma de tu API gráfica para texto alineado a la izquierda y centrado verticalmente
			graphics.DrawString(textRect, node->text, textColor);
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

	void TreeBoxReactor::Module::DrawNavigationLines(Graphics& graphics)
	{
		/*auto nodeHeight = m_window->ToScale(m_appearance->TreeItemHeight);
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
		}*/
	}

	void TreeBoxReactor::Module::Init()
	{
		m_root.isExpanded = true;
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

	TreeBoxItem TreeBoxReactor::Module::Insert(const TreeNodeHandle& key, const std::string& text)
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
	}

	TreeBoxItem TreeBoxReactor::Module::Find(const TreeNodeHandle& handle)
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
	}

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
		auto item = Find(handle);
		if (!item)
		{
			return;
		}

		Unlink(item.m_node);
		EraseNode(item.m_node);

		UpdateScrollData();

		GUI::UpdateWindow(m_window);
	}

	void TreeBoxReactor::Module::Erase(TreeBoxItem item)
	{
		Unlink(item.m_node);
		EraseNode(item.m_node);

		UpdateScrollData();

		GUI::UpdateWindow(m_window);
	}

	void TreeBoxReactor::Module::EraseNode(TreeNodeType* node)
	{
		if (node == nullptr)
			return;

		auto current = node->firstChild;
		while (current)
		{
			auto temp = current->nextSibling;
			EraseNode(current);

			current = temp;
		}
		node->m_lookup.clear();
		
		auto eraseResult = node->parent->m_lookup.erase(node->key);
	}

	void TreeBoxReactor::Module::Unlink(TreeNodeType* node)
	{
		auto current = node->parent->firstChild;
		while (current)
		{
			if (current == node)
			{
				auto prevSibling = current->prevSibling;
				if (current->nextSibling)
				{
					current->nextSibling->prevSibling = prevSibling;
				}
				if (current->prevSibling == nullptr)
				{
					node->parent->firstChild = current->nextSibling;
				}
				else
				{

					current->prevSibling->nextSibling = current->nextSibling;
				}
				break;
			}

			current = current->nextSibling;
		}
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
		
		int m_maxWidthEstimado = m_visibleWidths.empty() ? 0 : *m_visibleWidths.rbegin();
		
		Size contentSize;
		contentSize.Width = m_maxWidthEstimado == 0 ? clientArea.Width : (uint32_t)m_maxWidthEstimado;
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

	bool TreeBoxReactor::Module::Collapse(TreeBoxItem item)
	{
		if (!item || !item.m_node->firstChild)
		{
			return false;
		}
		bool needUpdate = item.m_node->isExpanded;
		item.m_node->isExpanded = false;

		return needUpdate;
	}

	bool TreeBoxReactor::Module::CollapseAll()
	{
		return CollapseAll({ &m_root, this });
	}

	bool TreeBoxReactor::Module::CollapseAll(TreeBoxItem item)
	{
		if (!item || !item.m_node->firstChild)
		{
			return false;
		}
		bool needUpdate = item.m_node != &m_root && item.m_node->isExpanded;
		if (item.m_node != &m_root)
			item.m_node->isExpanded = false;

		auto current = item.m_node->firstChild;

		while (current)
		{
			needUpdate |= CollapseAll({ current, this });
			current = current->nextSibling;
		}
		return needUpdate;
	}

	bool TreeBoxReactor::Module::ExpandAll()
	{
		return ExpandAll({ &m_root, this });
	}

	bool TreeBoxReactor::Module::ExpandAll(TreeBoxItem item)
	{
		if (!item || !item.m_node->firstChild)
		{
			return false;
		}
		bool needUpdate = item.m_node != &m_root && !item.m_node->isExpanded;
		if (item.m_node != &m_root) 
			item.m_node->isExpanded = true;
		
		auto current = item.m_node->firstChild;

		while (current)
		{
			needUpdate |= ExpandAll({ current, this });
			current = current->nextSibling;
		}
		return needUpdate;
	}

	bool TreeBoxReactor::Module::Expand(TreeBoxItem item)
	{
		if (!item || !item.m_node->firstChild)
		{
			return false;
		}
		bool needUpdate = !item.m_node->isExpanded;
		item.m_node->isExpanded = true;
		
		return needUpdate;
	}

	void TreeBoxReactor::Module::CollectVisibleDescendants(TreeNodeType* parent, int parentLevel, std::vector<FlatNode>& outList)
	{
		auto child= parent->firstChild;
		while (child != nullptr)
		{
			outList.push_back({ child, parentLevel + 1 });
			int totalWidth = CalculateNodeWidth(child, parentLevel + 1);
			m_visibleWidths.insert(totalWidth);
			
			if (child->isExpanded) 
			{
				CollectVisibleDescendants(child, parentLevel + 1, outList);
			}
			child = child->nextSibling;
		}
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
		if (node->isExpanded) return;
		node->isExpanded = true;

		auto it = std::find_if(m_flatVisibleTree.begin(), m_flatVisibleTree.end(),
							   [node](const FlatNode& fn) { return fn.Node == node; });

		if (it == m_flatVisibleTree.end()) 
		{
			return;
		}
		
		std::vector<FlatNode> descendantsToInsert;
		CollectVisibleDescendants(node, it->Level, descendantsToInsert);
        
		m_flatVisibleTree.insert(it + 1, descendantsToInsert.begin(), descendantsToInsert.end());
		/*int totalWidth = CalculateNodeWidth(child, childLevel, graphics);
		m_visibleWidths.insert(totalWidth);*/
		
		m_needsRecalculate = true;
	}

	int TreeBoxReactor::Module::CalculateNodeWidth(TreeNodeType* node, int level)
	{
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		if (node->visualWidth == -1) // Solo medimos si nunca se ha medido
		{
			int textWidth = m_graphics->GetTextExtent(node->text).Width;
			//int iconWidth = m_appearance->ExpanderButtonSize + 5; // padding
			node->visualWidth = textWidth /*+ iconWidth*/;
		}
		return node->visualWidth + (level * (int)appearance->DepthWidthMultiplier);
	}

	void TreeBoxReactor::Module::RebuildFlatTree()
	{
		m_flatVisibleTree.clear();
		m_visibleWidths.clear();

		auto child = m_root.firstChild;
		while (child != nullptr)
		{
			CollectVisibleNodes(child, 0);
			child = child->nextSibling;
		}

		m_needsRecalculate = true;
		m_needsRepaint = true;
	}

	void TreeBoxReactor::Module::CollectVisibleNodes(TreeNodeType* node, int level)
	{
		auto appearance = reinterpret_cast<TreeBoxAppearance*>(m_window->Appearance.get());
		m_flatVisibleTree.push_back({ node, level });

		int estimatedTextWidth = (int)m_graphics->GetTextExtent(node->text).Width;
		int totalWidth = (level * appearance->DepthWidthMultiplier) + 
						 appearance->ExpanderButtonSize + 5 + estimatedTextWidth;
		m_visibleWidths.insert(totalWidth);

		if (node->isExpanded)
		{
			auto child = node->firstChild;
			while (child)
			{
				CollectVisibleNodes(child, level + 1);
				child = child->nextSibling;
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

	void TreeBoxReactor::Module::SetText(TreeNodeType* node, const std::string& newText) const
	{
		if (node->text == newText)
			return;

		node->text = newText;

		if (true)
		{
			GUI::UpdateWindow(m_window);
		}
	}

	void TreeBoxItem::Collapse()
	{
		if (m_module->Collapse(*this))
		{
			m_module->Update();
			m_module->Draw();
		}
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

	TreeNodeType* TreeBoxReactor::Module::GetRoot()
	{
		return &m_root;
	}

	std::string TreeBoxReactor::Module::GetKeyPath(TreeBoxItem item, char separator)
	{
		std::string path;
		std::string temp;

		auto current = item.m_node;
		auto root = GetRoot();
		while (current->parent != root)
		{
			temp = separator;
			temp += current->key;
			path.insert(0, temp);

			current = current->parent;
		}
		path.insert(0, current->key);
		return path;
	}

	std::vector<TreeBoxItem> TreeBoxReactor::Module::GetSelected()
	{
		std::vector<TreeBoxItem> selections;
		/*for (size_t i = 0; i < m_mouseSelection.m_selections.size(); i++)
		{
			selections.emplace_back(TreeBoxItem{ m_mouseSelection.m_selections[i], this });
		}*/
		return selections;
	}

	bool TreeBoxReactor::Module::ShowNavigationLines(bool visible)
	{
		if (m_showNavigationLines == visible)
			return false;

		m_showNavigationLines = visible;
		return true;
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
										 [anchor](const FlatNode& f) { return f.Node == anchor; });
			auto itCurrent = std::find_if(m_flatVisibleTree.begin(), m_flatVisibleTree.end(), 
										  [current](const FlatNode& f) { return f.Node == current; });

			if (itAnchor == m_flatVisibleTree.end() || itCurrent == m_flatVisibleTree.end())
			{
				range.push_back(const_cast<TreeNodeType*>(current));
				return range;
			}

			auto start = std::distance(m_flatVisibleTree.begin(), itAnchor);
			auto end = std::distance(m_flatVisibleTree.begin(), itCurrent);
			if (start > end) std::swap(start, end);

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
		GetReactor().GetModule().Clear();
	}

	void TreeBox::CollapseAll()
	{
		if (GetReactor().GetModule().CollapseAll())
		{
			GetReactor().GetModule().Update();
			GetReactor().GetModule().Draw();
		}
	}

	void TreeBox::CollapseAll(TreeBoxItem item)
	{
		if (GetReactor().GetModule().CollapseAll(item))
		{
			GetReactor().GetModule().Update();
			GetReactor().GetModule().Draw();
		}
	}

	void TreeBox::Erase(const TreeNodeHandle& key)
	{
		GetReactor().GetModule().Erase(key);
	}

	void TreeBox::Erase(TreeBoxItem item)
	{
		GetReactor().GetModule().Erase(item);
	}

	TreeBoxItem TreeBox::Find(const TreeNodeHandle& key)
	{
		return GetReactor().GetModule().Find(key);
	}

	TreeBoxItem TreeBox::Insert(const TreeNodeHandle& key, const std::string& text)
	{
		auto& module = GetReactor().GetModule();
		auto item = module.Insert(key, text);

		if (item)
		{
			module.RebuildFlatTree();
			module.Update();
			module.Draw();
		}
		return item;
	}

	TreeBoxItem TreeBox::Insert(TreeBoxItem parent, const TreeNodeHandle& key, const std::string& text)
	{
		auto& module = GetReactor().GetModule();
		auto item = module.Insert(key, text, parent.GetNode());

		if (item)
		{
			module.Update();
			module.Draw();
		}
		return item;
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
		if (module.ExpandAll())
		{
			module.Update();
			module.Draw();
		}
	}

	void TreeBox::ExpandAll(TreeBoxItem item)
	{
		auto& module = GetReactor().GetModule();
		if (module.ExpandAll(item))
		{
			module.Update();
			module.Draw();
		}
	}

	std::string TreeBox::GetKeyPath(TreeBoxItem item, char separator)
	{
		return GetReactor().GetModule().GetKeyPath(item, separator);
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
		if (GetReactor().GetModule().ShowNavigationLines(visible))
		{
			GetReactor().GetModule().Update();
			GetReactor().GetModule().Draw();
		}
	}
}
