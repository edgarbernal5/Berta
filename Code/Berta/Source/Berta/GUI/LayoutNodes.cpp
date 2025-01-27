/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "LayoutNodes.h"

namespace Berta
{
	LayoutNode::LayoutNode(const std::string& id) : 
		m_id(id)
	{
	}

	ContainerLayout::ContainerLayout(bool isVertical) :
		m_isVertical(isVertical)
	{
	}

	void ContainerLayout::AddChild(std::unique_ptr<LayoutNode>&& child)
	{
		m_children.push_back(std::move(child));
	}
}