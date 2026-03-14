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
#include <any>

#include "Berta/GUI/ScrollableView.h"

namespace Berta
{
	using TreeNodeHandle = std::string;
	struct TreeBoxItem;

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
			: key(key_), text(text_), parent(parent_) {
		}

		TreeNodeType* Add(const TreeNodeHandle& childKey, const std::string& text_, TreeNodeType* parent_ = nullptr);
		TreeNodeType* Find(const TreeNodeHandle& key);

		bool isExpanded{ false };
		bool isSelected{ false };
		std::string text;
		TreeNodeHandle key;
		Image icon;
		Size textExtents;
		std::any userData;

		std::unordered_map<std::string, std::unique_ptr<TreeNodeType>> m_lookup;

		TreeNodeType* parent{ nullptr };
		TreeNodeType* firstChild{ nullptr };
		TreeNodeType* nextSibling{ nullptr };
		TreeNodeType* prevSibling{ nullptr };
	};

	class TreeBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control, Graphics* graphics) override;
		void Update(Graphics& graphics) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		void DblClick(Graphics& graphics, const ArgMouse& args) override;
		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
		void KeyReleased(Graphics& graphics, const ArgKeyboard& args) override;

		enum class InteractionArea : uint8_t
		{
			None,
			Node,
			Expander,
			Blank
		};

		struct Module
		{
			void CalculateVisibleNodes();
			void GetNodesInBetween(int startIndex, int endIndex, std::vector< TreeNodeType*>& nodes) const;
			uint32_t CalculateTreeSize(TreeNodeType* node);
			uint32_t CalculateNodeDepth(TreeNodeType* node);
			
			void Clear();
			TreeNodeType* GetNextVisible(TreeNodeType* node);
			
			int LocateNodeIndexInTree(TreeNodeType* node) const;
			TreeNodeType* LocateNodeIndexInTree(int nodeIndex) const;
			InteractionArea DetermineHoverArea(const Point& mousePosition);
			
			void Update();
			void Draw();
			void DrawTreeNodes(Graphics& graphics);
			void DrawNavigationLines(Graphics& graphics);
			void Init();
			void EnableMultiselection(bool enabled);

			TreeNodeHandle CleanKey(const TreeNodeHandle& key);
			
			TreeBoxItem Insert(const TreeNodeHandle& key, const std::string& text);
			TreeBoxItem Insert(const TreeNodeHandle& key, const std::string& text, TreeNodeType* parentNode);
			TreeBoxItem Find(const TreeNodeHandle& handle);
			TreeNodeHandle GenerateUniqueHandle(const std::string& key, TreeNodeType* parentNode);
			
			void Erase(const TreeNodeHandle& handle);
			void Erase(TreeBoxItem item);
			void EraseNode(TreeNodeType* node);
			void Unlink(TreeNodeType* node);
			void UpdateScrollData();

			bool IsVisibleNode(TreeNodeType* node) const;
			bool IsVisibleNode(TreeNodeType* node, int& visibleIndex) const;
			bool IsAnySiblingVisible(TreeNodeType* node) const;
			void EmitSelectionEvent();
			void EmitExpansionEvent(TreeNodeType* node);

			bool Collapse(TreeBoxItem item);
			bool CollapseAll();
			bool CollapseAll(TreeBoxItem item);
			bool ExpandAll();
			bool ExpandAll(TreeBoxItem item);
			bool Expand(TreeBoxItem item);

			void SetIcon(TreeNodeType* node, const Image& icon);
			void SetText(TreeNodeType* node, const std::string& newText) const;

			TreeNodeType* GetRoot();
			std::string GetKeyPath(TreeBoxItem item, char separator);
			std::vector<TreeBoxItem> GetSelected();

			bool ShowNavigationLines(bool visible);

			void InitScrollableView();
			
			TreeNodeType m_root;
			Window* m_window{ nullptr };
			ControlBase* m_control{ nullptr };
			std::vector<TreeNodeType*> m_visibleNodes;
			bool m_drawImages{ false };

			InteractionArea m_hoveredArea{ InteractionArea::None};
			InteractionArea m_pressedArea{ InteractionArea::None };

			std::unique_ptr<ScrollableView> m_scrollableView;
			
			bool m_multiselection{ true };
			bool m_shiftPressed{ false };
			bool m_ctrlPressed{ false };
			bool m_showNavigationLines{ true };
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
			return { m_node->firstChild, m_module};
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
