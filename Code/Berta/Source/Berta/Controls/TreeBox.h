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

namespace Berta
{
	struct NodeType
	{
		bool expanded{ false };
		std::string text;
		NodeType* parent;
		std::vector<NodeType*> children;
	};

	class TreeBoxReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		struct Module
		{
			void Clear();
			void Insert(const std::string& key, const std::string& text);
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	class TreeBox : public Control<TreeBoxReactor>
	{
	public:
		TreeBox() = default;
		TreeBox(Window* parent, const Rectangle& rectangle);

		void Clear();
		void Insert(const std::string& key, const std::string& text);
	};
}

#endif
