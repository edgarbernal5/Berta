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

	TreeBoxItem TreeBoxReactor::Module::Insert(const std::string& key, const std::string& text)
	{
		TreeNodeType* parentNode{ nullptr };

		return {};
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const std::string& key, const std::string& text, const TreeNodeHandle& parentHandle)
	{
		TreeNodeType* parentNode{ nullptr };
		if (!parentHandle.empty()) {
			auto it = nodeLookup.find(parentHandle);
			if (it == nodeLookup.end()) {
				return {};
			}
			parentNode = it->second;
		}

		std::string handle = GenerateUniqueHandle(key, parentNode);
		auto node = std::make_unique<TreeNodeType>(handle, text, parentNode);
		TreeNodeType* nodePtr = node.get();

		if (parentNode) {
			//parentNode->children.emplace_back(std::move(node));
		}
		//else {
		//	rootNodes.push_back(std::move(node));
		//}

		nodeLookup[handle] = nodePtr;
		return {};
	}

	TreeBoxItem TreeBoxReactor::Module::Find(const TreeNodeHandle& handle)
	{
		return {};
	}

	TreeNodeHandle TreeBoxReactor::Module::GenerateUniqueHandle(const std::string& key, TreeNodeType* parentNode)
	{
		if (!parentNode) {
			return key;
		}
		return parentNode->key + "/" + key;
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

	TreeBoxItem TreeBox::Insert(const std::string& key, const std::string& text)
	{
		return m_reactor.GetModule().Insert(key, text);
	}
}
