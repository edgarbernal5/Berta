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

		struct TreeNodeType
		{
			TreeNodeType(const std::string& key_, const std::string& text_, TreeNodeType* parent_ = nullptr)
				: key(key_),text(text_), parent(parent_) {}

			bool expanded{ false };
			bool selected{ false };
			std::string text;
			std::string key;

			TreeNodeType* parent;
			std::vector<TreeNodeType*> children;
		};

		struct Module
		{
			void Clear();
			TreeBoxItem Insert(const std::string& key, const std::string& text);
			TreeBoxItem Insert(const std::string& key, const std::string& text, const TreeNodeHandle& parentHandle);
			TreeBoxItem Find(const TreeNodeHandle& handle);
			TreeNodeHandle GenerateUniqueHandle(const std::string& text, TreeNodeType* parentNode);

			void Erase(const TreeNodeHandle& handle);

			std::unordered_map<std::string, TreeNodeType*> nodeLookup;
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	struct TreeBoxItem
	{

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
