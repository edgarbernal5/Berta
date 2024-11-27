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

namespace Berta
{
	void TreeBoxReactor::Init(ControlBase& control)
	{
		m_control = &control;
	}

	void TreeBoxReactor::Update(Graphics& graphics)
	{
		BT_CORE_TRACE << " -- TreeBox Update() " << std::endl;
		
	}

	void TreeBoxReactor::Module::Insert(const std::string& key, const std::string& text)
	{
	}

	TreeNodeType* TreeBoxReactor::Module::Find(const TreeNodeHandle& handle)
	{
		return nullptr;
	}

	void TreeBoxReactor::Module::Erase(const TreeNodeHandle& handle)
	{
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
		m_reactor.GetModule().Clear();
	}

	void TreeBox::Insert(const std::string& key, const std::string& text)
	{
		m_reactor.GetModule().Insert(key, text);
	}
}
