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

	class TreeBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;

		struct TreeNodeType
		{
			TreeNodeType() = default;
			TreeNodeType(const std::string& key_, const std::string& text_, TreeNodeType* parent_ = nullptr)
				: key(key_),text(text_), parent(parent_) {}

			bool isExpanded{ false };
			bool selected{ false };
			std::string text;
			std::string key;
			Image icon;

			TreeNodeType* parent{ nullptr };
			TreeNodeType* firstChild{ nullptr };
			TreeNodeType* nextSibling{ nullptr };
		};

		struct ViewportData
		{
			Rectangle m_backgroundRect{};
			bool m_needVerticalScroll{ false };
			bool m_needHorizontalScroll{ false };
			Size m_contentSize{};
		};

		struct Module
		{
			void CalculateViewport(ViewportData& viewportData);
			void CalculateVisibleNodes();
			uint32_t CalculateTreeSize(TreeNodeType* node);
			uint32_t CalculateNodeDepth(TreeNodeType* node);
			void Clear();
			TreeNodeType* GetNextVisible(TreeNodeType* node);
			TreeBoxItem Insert(const std::string& key, const std::string& text);
			TreeBoxItem Insert(const std::string& key, const std::string& text, const TreeNodeHandle& parentHandle);
			TreeBoxItem Find(const TreeNodeHandle& handle);
			TreeNodeHandle GenerateUniqueHandle(const std::string& text, TreeNodeType* parentNode);
			void Erase(const TreeNodeHandle& handle);
			bool UpdateScrollBars();

			std::unordered_map<std::string, std::unique_ptr<TreeNodeType>> m_nodeLookup;
			
			Point m_scrollOffset{};
			std::unique_ptr<ScrollBar> m_scrollBarVert;
			std::unique_ptr<ScrollBar> m_scrollBarHoriz;

			ViewportData m_viewport;
			TreeNodeType m_root;
			Window* m_window{ nullptr };
			std::vector<TreeNodeType*> m_visibleNodes;
			bool m_drawImages{ false };
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	struct TreeBoxItem
	{
		TreeBoxItem() = default;
		TreeBoxItem(TreeBoxReactor::TreeNodeType* node) : m_node(node) {}

	private:
		TreeBoxReactor::TreeNodeType* m_node{ nullptr };
	};

	class TreeBox : public Control<TreeBoxReactor>
	{
	public:
		TreeBox() = default;
		TreeBox(Window* parent, const Rectangle& rectangle);

		void Clear();
		TreeBoxItem Insert(const std::string& key, const std::string& text);
	};
}

#endif
