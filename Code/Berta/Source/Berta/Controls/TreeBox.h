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

#include <string>
#include <vector>
#include <unordered_map>

namespace Berta
{
	using TreeNodeHandle = std::string;
	struct TreeBoxItem;

	struct TreeBoxAppearance : public ControlAppearance
	{
		uint32_t ExpanderSize = 12u;
	};

	class TreeBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
		void KeyReleased(Graphics& graphics, const ArgKeyboard& args) override;

		struct TreeNodeType
		{
			TreeNodeType() = default;
			TreeNodeType(const TreeNodeHandle& key_, const std::string& text_, TreeNodeType* parent_ = nullptr)
				: key(key_),text(text_), parent(parent_) {}

			bool isExpanded{ false };
			bool isSelected{ false };
			std::string text;
			TreeNodeHandle key;
			Image icon;

			TreeNodeType* parent{ nullptr };
			TreeNodeType* firstChild{ nullptr };
			TreeNodeType* nextSibling{ nullptr };
			TreeNodeType* prevSibling{ nullptr };
		};

		enum class InteractionArea
		{
			None,
			Node,
			Expander,
			Blank
		};

		struct ViewportData
		{
			Rectangle m_backgroundRect{};
			bool m_needVerticalScroll{ false };
			bool m_needHorizontalScroll{ false };
			Size m_contentSize{};

			int m_startingVisibleIndex{ -1 };
			int m_endingVisibleIndex{ -1 };
		};

		struct MouseSelection
		{
			bool IsSelected(TreeNodeType* node) const;

			void Select(TreeNodeType* node);
			void Deselect(TreeNodeType* node);

			std::vector<TreeNodeType*> m_selections;
			TreeNodeType* m_pressedNode{ nullptr };
			TreeNodeType* m_hoveredNode{ nullptr };
			TreeNodeType* m_selectedNode{ nullptr };
			bool m_inverseSelection{ false };
		};

		struct Module
		{
			void CalculateViewport(ViewportData& viewportData);
			void CalculateVisibleNodes();
			uint32_t CalculateTreeSize(TreeNodeType* node);
			uint32_t CalculateNodeDepth(TreeNodeType* node);
			void Clear();
			TreeNodeType* GetNextVisible(TreeNodeType* node);
			int LocateNodeIndexInTree(TreeNodeType* node);
			TreeNodeType* LocateNodeIndexInTree(int nodeIndex);
			InteractionArea DetermineHoverArea(const Point& mousePosition);
			void DrawTreeNodes(Graphics& graphics);
			void DrawNavigationLines(Graphics& graphics);
			void Init();
			void GenerateNavigationLines();
			
			TreeBoxItem Insert(const TreeNodeHandle& key, const std::string& text);
			TreeBoxItem Insert(const TreeNodeHandle& key, const std::string& text, const TreeNodeHandle& parentHandle);
			TreeBoxItem Find(const TreeNodeHandle& handle);
			TreeNodeHandle GenerateUniqueHandle(const std::string& text, TreeNodeType* parentNode);
			void Erase(const TreeNodeHandle& handle);
			void Erase(TreeBoxItem item);
			void EraseNode(TreeNodeType* node);
			void Unlink(TreeNodeType* node);
			bool UpdateScrollBars();
			bool ClearSingleSelection();
			void SelectItem(TreeNodeType* node);
			bool UpdateSingleSelection(TreeNodeType* node);
			bool IsVisibleNode(TreeNodeType* node) const;
			bool IsVisibleNode(TreeNodeType* node, int& visibleIndex) const;
			bool IsAnySiblingVisible(TreeNodeType* node) const;
			void EmitSelectionEvent();
			void EmitExpansionEvent(TreeNodeType* node);

			bool ExpandAll();
			bool ExpandAll(TreeBoxItem item);

			void SetIcon(TreeNodeType* node, const Image& icon);
			void SetText(TreeNodeType* node, const std::string& newText) const;

			std::vector<TreeBoxItem> GetSelected();
			std::unordered_map<TreeNodeHandle, std::unique_ptr<TreeNodeType>> m_nodeLookup;
			
			Point m_scrollOffset{};
			std::unique_ptr<ScrollBar> m_scrollBarVert;
			std::unique_ptr<ScrollBar> m_scrollBarHoriz;

			ViewportData m_viewport;
			TreeNodeType m_root;
			Window* m_window{ nullptr };
			TreeBoxAppearance* m_appearance{ nullptr };
			std::vector<TreeNodeType*> m_visibleNodes;
			bool m_drawImages{ false };

			InteractionArea m_hoveredArea{ InteractionArea::None};
			InteractionArea m_pressedArea{ InteractionArea::None };

			MouseSelection m_mouseSelection;
			bool m_multiselection{ false };
			bool m_shiftPressed{ false };
			bool m_ctrlPressed{ false };
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	struct TreeBoxItem
	{
		TreeBoxItem() = default;
		TreeBoxItem(TreeBoxReactor::TreeNodeType* node, TreeBoxReactor::Module* module) : m_node(node), m_module(module) {}
		
		void SetText(const std::string& text)
		{
			m_module->SetText(m_node, text);
		}

		void SetIcon(const Image& icon)
		{
			m_module->SetIcon(m_node, icon);
		}

		TreeNodeHandle& GetHandle()
		{
			return m_node->key;
		}

		operator bool() const
		{
			return m_node;
		}

		friend struct TreeBoxReactor::Module;
	private:
		TreeBoxReactor::TreeNodeType* m_node{ nullptr };
		TreeBoxReactor::Module* m_module{ nullptr };
	};

	struct ArgTreeBox
	{
		TreeBoxItem &Item;
		bool Expanded{ false };

		ArgTreeBox(TreeBoxItem item, bool expanded) : Item(item), Expanded(expanded){}
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
		TreeBox(Window* parent, const Rectangle& rectangle);

		void Clear();
		void Erase(const TreeNodeHandle& key);
		void Erase(TreeBoxItem item);
		TreeBoxItem Find(const TreeNodeHandle& key);
		TreeBoxItem Insert(const TreeNodeHandle& key, const std::string& text);
		TreeBoxItem Insert(TreeBoxItem parent, const TreeNodeHandle& key, const std::string& text);
		void ExpandAll();
		void ExpandAll(TreeBoxItem item);

		std::vector<TreeBoxItem> GetSelected();
	};
}

#endif
