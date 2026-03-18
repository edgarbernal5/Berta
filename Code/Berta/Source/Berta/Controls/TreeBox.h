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
	using TreeNodeHandle = std::string;
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
		TreeNodeType(const TreeNodeHandle& key_, const std::string& text_, TreeNodeType* parent_ = nullptr)
			: key(key_), text(text_), parent(parent_)
		{
		}
		~TreeNodeType() = default;
		
		//TreeNodeType* Add(const TreeNodeHandle& childKey, const std::string& text_, TreeNodeType* parent_ = nullptr);
		//TreeNodeType* Find(const TreeNodeHandle& key);

		std::string text;
		TreeNodeHandle key;
		Image icon;
		std::any userData;
		bool isExpanded{ false };
		int cachedTextWidth { -1 };
		
		TreeNodeType* parent{ nullptr };
		std::vector<TreeNodeType*> children;
	};
	
	struct FlatNode 
	{
		TreeNodeType* Node;
		int Level;
		bool IsLastChild;
		uint32_t VerticalLineMask; // Bits: 1 = dibujar línea vertical, 0 = espacio vacío
		
		FlatNode(TreeNodeType* node, int level, bool isLastChild, uint32_t vertLineMask) : 
			Node(node), Level(level), IsLastChild(isLastChild), VerticalLineMask(vertLineMask)
		{}
	};

	class TreeModel
	{
	public:
        TreeModel()
        {
            m_root = m_nodePool.Allocate("$$ROOT$$", "", nullptr);
            m_root->isExpanded = true;
        }

        ~TreeModel()
        {
            Clear();
            m_nodePool.Deallocate(m_root);
        }

        TreeNodeType* GetRoot() const { return m_root; }

        TreeNodeType* Insert(const std::string& key, const std::string& text, TreeNodeType* parent = nullptr)
        {
            if (m_lookup.find(key) != m_lookup.end())
            {
	            return m_lookup[key];
            }
            TreeNodeType* actualParent = parent ? parent : m_root;
            
            TreeNodeType* newNode = m_nodePool.Allocate(key, text, actualParent);
            
            actualParent->children.emplace_back(newNode);
            m_lookup[key] = newNode;

            return newNode;
        }

		TreeNodeType* Find(const TreeNodeHandle& key) const
        {
        	auto it = m_lookup.find(key);
        	return (it != m_lookup.end()) ? it->second : nullptr;
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
        	m_lookup.clear();
        }
		
		std::string GetKeyPath(TreeNodeType* node, char separator) const
        {
        	if (!node || node == m_root) return "";

        	std::string path = node->key;
        	TreeNodeType* current = node->parent;

        	while (current && current != m_root)
        	{
        		path = current->key + separator + path; //TODO
        		current = current->parent;
        	}

        	return path;
        }

		void MoveNode(TreeNodeType* nodeToMove, TreeNodeType* targetNode, DropPosition pos)
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

            m_lookup.erase(node->key);
            m_nodePool.Deallocate(node);
        }
		
		ObjectPool<TreeNodeType, 1024> m_nodePool;
		std::unordered_map<std::string, TreeNodeType*> m_lookup;
		TreeNodeType* m_root { nullptr };
    };
	
	class TreeBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control, Graphics* graphics) override;
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

		struct Module
		{
			void Update();
			void Draw();
			void DrawTreeNodes(Graphics& graphics);
			
			void EnableMultiselection(bool enabled);

			TreeNodeHandle CleanKey(const TreeNodeHandle& key);
			
			//TreeBoxItem Insert(const TreeNodeHandle& key, const std::string& text);
			//TreeBoxItem Insert(const TreeNodeHandle& key, const std::string& text, TreeNodeType* parentNode);
			//TreeBoxItem Find(const TreeNodeHandle& handle);
			TreeNodeHandle GenerateUniqueHandle(const TreeNodeHandle& key, TreeNodeType* parentNode);
			
			void Erase(const TreeNodeHandle& handle);
			void Erase(TreeBoxItem item);
			void EraseNode(TreeNodeType* node);
			void UpdateScrollData();

			void EmitSelectionEvent();
			void EmitExpansionEvent(TreeNodeType* node);

			bool Expand(TreeBoxItem item);
			
			void CollapseNode(TreeNodeType* node);
			void ExpandNode(TreeNodeType* node);
			
			int CalculateNodeWidth(TreeNodeType* node, int level);
			
			void RebuildFlatTree();
			void CollectVisibleNodes(TreeNodeType* node, int level, uint32_t lineMask, bool isLastChild);
			
			void SetIcon(TreeNodeType* node, const Image& icon);
			void SetText(TreeNodeType* node, const std::string& newText);

			std::vector<TreeBoxItem> GetSelected();

			bool ShowNavigationLines(bool visible);

			bool IsDescendantOf(TreeNodeType* node, TreeNodeType* potentialAncestor) const;
			void InitScrollableView();
			
			TreeModel m_model;
			std::unique_ptr<ScrollableView> m_scrollableView;
			SelectionController<TreeNodeType*> m_selectionController;
			
			std::vector<FlatNode> m_flatVisibleTree;
			std::multiset<int> m_visibleWidths;
			
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
			bool m_multiselection{ true };
			bool m_shiftPressed{ false };
			bool m_ctrlPressed{ false };
			bool m_showNavigationLines{ true };
			
			bool m_needsRepaint{ false };
			bool m_needsRecalculate{ false };
			
			SelectionController<TreeNodeType*>::RangeResolver m_treeRangeResolver;
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	struct TreeBoxItem
	{
		TreeBoxItem() = default;
		TreeBoxItem(TreeNodeType* node, TreeBoxReactor::Module* module) : m_node(node), m_module(module) {}
		
		void SetText(const std::string& text)
		{
			m_module->SetText(m_node, text);
		}

		void SetIcon(const Image& icon)
		{
			m_module->SetIcon(m_node, icon);
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

		std::string& GetText() const
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

		void Select();

		operator bool() const
		{
			return m_node;
		}

		friend struct TreeBoxReactor::Module;
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

	struct ArgTreeBoxSelection
	{
		std::vector<TreeBoxItem> Items;
	};

	struct TreeBoxEvents : public ControlEvents
	{
		Event<ArgTreeBox> Expanded;
		Event<ArgTreeBoxSelection> Selected;
	};

	class TreeBox : public Control<TreeBoxReactor, TreeBoxEvents, TreeBoxAppearance>
	{
	public:
		TreeBox() = default;
		TreeBox(Window* parent, const Rectangle& rectangle = {});

		void Clear();
		void CollapseAll();
		void CollapseAll(TreeBoxItem item);

		void Erase(const TreeNodeHandle& key);
		void Erase(TreeBoxItem item);
		TreeBoxItem Find(const TreeNodeHandle& key);
		TreeBoxItem Insert(const TreeNodeHandle& key, const std::string& text);
		TreeBoxItem Insert(TreeBoxItem parent, const TreeNodeHandle& key, const std::string& text);
		void DeselectAll();
		void ExpandAll();
		void ExpandAll(TreeBoxItem item);

		std::string GetKeyPath(TreeBoxItem item, char separator);
		std::vector<TreeBoxItem> GetSelected();

		void EnableMultiselection(bool enabled);

		void ShowNavigationLines(bool visible);
	};
}

#endif
