/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_TREE_BOX_HEADER
#define BT_TREE_BOX_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/ScrollBar.h"
#include "Berta/Paint/Image.h"

#include "Berta/GUI/ScrollableView.h"
#include "Berta/GUI/SelectionController.h"
#include "Berta/Core/ObjectPool.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <any>
#include <set>


namespace Berta
{
	using TreeNodeHandle = std::wstring;
	struct TreeBoxItem;
	struct TreeNodeType;
	
	struct TreeBoxAppearance : public ControlAppearance
	{
		uint32_t ExpanderButtonSize = 12u;
		uint32_t DepthWidthMultiplier = 20u;
		uint32_t TreeItemHeight = 20u;
	};
	
	struct TreeNodeType
	{
		TreeNodeType() = default;
		TreeNodeType(const TreeNodeHandle& key_, const std::wstring& text_, TreeNodeType* parent_ = nullptr)
			: key(key_), text(text_), parent(parent_)
		{
		}
		~TreeNodeType() = default;

		std::wstring text;
		TreeNodeHandle key; //relative key.
		CheckState checkState { CheckState::None };
		Image icon;
		std::any userData;
		bool isExpanded{ false };
		std::optional<uint32_t> cachedTextWidth { std::nullopt };
		
		TreeNodeType* parent{ nullptr };
		std::vector<TreeNodeType*> children;
	};
	
	struct FlatNode 
	{
		TreeNodeType* Node;
		uint32_t Level;
		uint32_t VerticalLineMask; // Bits: 1 = draw vertical line, 0 = empty space
		bool IsLastChild;
		
		FlatNode(TreeNodeType* node, uint32_t level, bool isLastChild, uint32_t vertLineMask) : 
			Node(node), Level(level), VerticalLineMask(vertLineMask), IsLastChild(isLastChild)
		{
		}
	};

	class TreeModel
	{
	public:
        TreeModel();
        ~TreeModel();

        TreeNodeType* GetRoot() const { return m_root; }

        TreeNodeType* Insert(const TreeNodeHandle& key, const std::wstring& text, TreeNodeType* parent = nullptr);

		TreeNodeType* Find(const TreeNodeHandle& absolutePath) const
        {
			if (absolutePath.empty() || !m_root) return nullptr;

			std::vector<std::wstring> pathParts = StringUtils::Split(absolutePath, '/');
			TreeNodeType* current = m_root;

			for (const std::wstring& part : pathParts)
			{
				bool found = false;
				for (auto* child : current->children)
				{
					if (child->key == part)
					{
						current = child;
						found = true;
						break;
					}
				}
        
				// Si en algún nivel no encontramos el hijo, la ruta no existe
				if (!found)
				{
					return nullptr;
				}
			}

			return current;
        }

		void Erase(TreeNodeType* node)
        {
        	if (!node || node == m_root) return;

        	if (node->parent)
        	{
        		auto& siblings = node->parent->children;
        		siblings.erase(std::remove(siblings.begin(), siblings.end(), node), siblings.end());
        	}

        	EraseRecursive(node);
        }

        void Clear()
        {
            for (auto* child : m_root->children)
            {
                EraseRecursive(child);
            }
            m_root->children.clear();
        }
		
		std::wstring GetKeyPath(TreeNodeType* node, wchar_t separator) const
        {
        	if (!node || node == m_root)
        	{
        		return L"";
        	}

        	std::vector<const std::wstring*> parts;
        	TreeNodeType* current = node;
        	size_t totalLength = 0;

        	while (current && current != m_root)
        	{
        		parts.push_back(&(current->key));
        		totalLength += current->key.length();
        		current = current->parent;
        	}

        	if (!parts.empty())
        	{
        		totalLength += parts.size() - 1;
        	}

        	std::wstring path;
        	path.reserve(totalLength);

        	for (auto it = parts.rbegin(); it != parts.rend(); ++it)
        	{
        		if (it != parts.rbegin())
        		{
        			path += separator;
        		}
        		path += **it;
        	}

        	return path;
        }

		void MoveNode(TreeNodeType* nodeToMove, TreeNodeType* targetNode, DropPosition pos);
		
	private:
		
        void EraseRecursive(TreeNodeType* node)
        {
            if (!node) return;
        	auto childrenCopy = node->children;
            for (auto* child : childrenCopy)
            {
                EraseRecursive(child);
            }

            if (node->parent)
            {
                auto& siblings = childrenCopy;
                siblings.erase(std::remove(siblings.begin(), siblings.end(), node), siblings.end());
            }

            m_nodePool.Deallocate(node);
        }
		
		ObjectPool<TreeNodeType, 1024> m_nodePool;
		TreeNodeType* m_root { nullptr };
    };
	
	class TreeBoxReactor : public ControlReactor
	{
	public:
		void Update(Graphics& graphics) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		void DblClick(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
		void KeyReleased(Graphics& graphics, const ArgKeyboard& args) override;
		void DpiChanged(Graphics& graphics) override;
		
		struct Module
		{
			void DrawTreeNodes(Graphics& graphics);
			
			void EnableMultiselection(bool enabled);
			void SelectNodes(const std::vector<TreeNodeType*>& nodes, bool append = false);
			
			TreeNodeHandle CleanKey(const TreeNodeHandle& key);
			TreeNodeHandle GenerateUniqueHandle(const TreeNodeHandle& key, TreeNodeType* parentNode);
			
			void ResetScrollOffset();
			void UpdateScrollData();

			void EmitSelectionEvent();
			void EmitExpansionEvent(TreeNodeType* node);
			
			void CollapseNode(TreeNodeType* node);
			void ExpandNode(TreeNodeType* node);
			
			uint32_t CalculateNodeWidth(TreeNodeType* node, uint32_t level);
			void InvalidateNodes(const std::vector<TreeNodeType*>& nodes);
			void ScrollToItem(TreeNodeType* node);
			
			void ResetDragState();
			void RebuildFlatTree(bool resetWidthCache = false);
			void CollectVisibleNodes(TreeNodeType* node, uint32_t level, uint32_t lineMask, bool isLastChild, bool resetWidthCache);

			bool ShowNavigationLines(bool visible);
			bool ShowIcons(bool visible);

			bool IsDescendantOf(TreeNodeType* node, TreeNodeType* potentialAncestor) const;
			void InitScrollableView();
			
			TreeModel m_model;
			std::unique_ptr<ScrollableView> m_scrollableView;
			SelectionController<TreeNodeType*> m_selectionController;
			
			std::vector<FlatNode> m_flatVisibleTree;
			std::multiset<uint32_t> m_visibleWidths;
			
			Window* m_window{ nullptr };
			Graphics* m_graphics{ nullptr };
			ControlBase* m_control{ nullptr };
			
			TreeNodeType* m_focusedNode{ nullptr };
			TreeNodeType* m_hoveredNode{ nullptr };
			
			// Estado del Drag & Drop
			bool m_allowDragAndDrop{ true };
			bool m_isDragging{ false };
			Point m_dragStartPoint{ 0, 0 };
        
			TreeNodeType* m_draggedNode{ nullptr };
			TreeNodeType* m_dropTargetNode{ nullptr };
			DropPosition m_dropPosition{ DropPosition::None };
			
			bool m_drawImages{ false };
			bool m_drawCheck{ false };
			bool m_multiselection{ true };
			bool m_showNavigationLines{ true };
			
			bool m_needsRecalculate{ false };
			
			SelectionController<TreeNodeType*>::RangeResolver m_treeRangeResolver;
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	protected:
		void DoOnInit() override;
		
	private:
		Module m_module;
	};

	struct TreeBoxItem
	{
		TreeBoxItem() = default;
		TreeBoxItem(TreeNodeType* node, TreeBoxReactor::Module* module) : m_node(node), m_module(module) {}
		
		void SetText(const std::wstring& text)
		{
			if (!m_node || m_node->text == text)
			{
				return;
			}
			
			m_node->text = text;
			m_node->cachedTextWidth = -1;
			m_module->RebuildFlatTree();
		
			GUI::MarkAsNeedUpdate(m_module->m_window);
		}

		void SetIcon(const Image& icon)
		{
			if (!m_node)
			{
				return;
			}
			m_node->icon = icon;
			
			GUI::MarkAsNeedUpdate(m_module->m_window);
		}
		
		void SetChecked(bool checked)
		{
			if (!m_node || m_node->checkState == CheckState::None)
			{
				return;
			}
			CheckState newState = checked ? CheckState::Checked : CheckState::Unchecked;
			m_node->checkState = newState;
			
			//PropagateCheckStateToChildren(m_node, newState);
			//UpdateAncestorsCheckState(m_node);
			
			m_module->m_drawCheck = true;
			
			GUI::MarkAsNeedUpdate(m_module->m_window);
		}

		template<typename T>
		bool HasUserData() const
		{
			return std::any_cast<T>(&UserData());
		}
		
		template<typename T>
		const T& GetUserData() const
		{
			auto p = std::any_cast<T>(&UserData());
			return *p;
		}

		template<typename T>
		TreeBoxItem& SetUserData(T&& userData)
		{
			UserData() = std::forward<T>(userData);
			return *this;
		}

		void Collapse();
		void Expand();

		std::wstring& GetText() const
		{
			return m_node->text;
		}

		TreeNodeHandle& GetHandle() const
		{
			return m_node->key;
		}

		TreeNodeType* GetNode() const
		{
			return m_node;
		}

		TreeBoxItem FirstChild()
		{
			return { m_node->children[0], m_module};
		}

		void Select(bool ctrlPressed = false, bool shiftPressed = false);
		void ScrollToItem();
		
		operator bool() const
		{
			return m_node;
		}

	private:
		std::any& UserData();
		const std::any& UserData() const;
		
		TreeNodeType* m_node{ nullptr };
		TreeBoxReactor::Module* m_module{ nullptr };
	};

	struct ArgTreeBox
	{
		TreeBoxItem &Item;
		bool IsExpanded{ false };

		ArgTreeBox(TreeBoxItem item, bool isExpanded) : Item(item), IsExpanded(isExpanded){}
	};
	
	struct ArgTreeDragDrop
	{
		TreeBoxItem DraggedItem;
		TreeBoxItem TargetItem;
		DropPosition Position;
		bool Cancel{ false };
	};

	struct ArgTreeBoxSelection
	{
		std::vector<TreeBoxItem> Items;
	};

	struct TreeBoxEvents : public ControlEvents
	{
		Event<ArgTreeDragDrop> DragStart;
		Event<ArgTreeDragDrop> DragOver;
		Event<ArgTreeDragDrop> BeforeDrop;
		Event<ArgTreeDragDrop> NodeMoved;
		Event<ArgTreeBox> Expanded;
		Event<ArgTreeBoxSelection> Selected;
	};

	class TreeBox : public Control<Category::ControlTag, TreeBoxReactor, TreeBoxEvents, TreeBoxAppearance>
	{
	public:
		TreeBox() = default;
		TreeBox(Window* parent, const Rectangle& rectangle = {});

		void Clear();
		
		void CollapseAll();
		void CollapseAll(TreeBoxItem item);

		void SelectItems(const std::vector<TreeBoxItem>& nodes);
		void DeselectAll();
		
		TreeBoxItem Find(const TreeNodeHandle& key);
		TreeBoxItem Insert(const TreeNodeHandle& absoluteKey, const std::wstring& text);
		TreeBoxItem Insert(TreeBoxItem parent, const TreeNodeHandle& key, const std::wstring& text);
		
		void Erase(const TreeNodeHandle& key);
		void Erase(TreeBoxItem item);
		
		void ExpandAll();
		void ExpandAll(TreeBoxItem item);
		
		void ScrollToItem(TreeBoxItem item);
		
		std::wstring GetKeyPath(TreeBoxItem item, wchar_t separator);
		std::vector<TreeBoxItem> GetSelected();

		void Filter(const std::wstring& text);
		
		void EnableMultiselection(bool enabled);
		void ShowNavigationLines(bool visible);
		void ShowIcons(bool visible);
	};
}

#endif
