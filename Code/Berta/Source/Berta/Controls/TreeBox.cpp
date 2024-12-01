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
		
		auto currentNode = m_module.m_firstVisible;
		while (currentNode != nullptr)
		{
			currentNode = m_module.GetNextVisible(currentNode);
		}
	}

	void TreeBoxReactor::Module::CalculateFirstVisible()
	{
		if (m_root.firstChild == nullptr)
		{
			m_firstVisible = nullptr;
			return;
		}

		m_firstVisible = m_root.firstChild;
	}

	Berta::TreeBoxReactor::TreeNodeType* TreeBoxReactor::Module::GetNextVisible(TreeNodeType* node)
	{
		if (node == nullptr)
			return nullptr;

		if (node->expanded)
		{
			if (node->firstChild == nullptr)
				return nullptr;

			return node->firstChild;
		}

		if (node->nextSibling != nullptr)
		{
			return node->nextSibling;
		}

		auto parent = node->parent;
		if (parent == &m_root)
			return nullptr;

		//GetNextVisible(parent);

		return nullptr;
	}

	void TreeBoxReactor::Module::Clear()
	{
		m_firstVisible = nullptr;
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const std::string& key, const std::string& text)
	{
		auto hasParentIndex = key.find_last_of('/');
		if (hasParentIndex != std::string::npos)
		{
			return Insert(key.substr(hasParentIndex, key.size()), text, key.substr(0, hasParentIndex));
		}

		auto node = std::make_unique<TreeNodeType>(key, text, &m_root);
		TreeNodeType* nodePtr = node.get();

		m_root.firstChild = nodePtr;
		
		nodeLookup[key] = std::move(node);
		return { nodePtr };
	}

	TreeBoxItem TreeBoxReactor::Module::Insert(const std::string& key, const std::string& text, const TreeNodeHandle& parentHandle)
	{
		TreeNodeType* parentNode{ nullptr };
		if (!parentHandle.empty())
		{
			auto it = nodeLookup.find(parentHandle);
			if (it == nodeLookup.end())
			{
				return {};
			}
			parentNode = it->second.get();
		}

		std::string handle = GenerateUniqueHandle(key, parentNode);
		auto node = std::make_unique<TreeNodeType>(handle, text, parentNode);
		node->parent = parentNode;
		TreeNodeType* nodePtr = node.get();

		if (parentNode)
		{
			if (parentNode->firstChild == nullptr)
			{
				parentNode->firstChild = nodePtr;
			}
			else
			{
				auto where = parentNode->firstChild;
				while (where->nextSibling != nullptr)
				{
					where = where->nextSibling;
				}
				where->nextSibling = nodePtr;
			}
		}
		else
		{
			if (m_root.firstChild == nullptr)
			{
				m_root.firstChild = nodePtr;
			}
			else
			{
				auto where = m_root.firstChild;
				while (where->nextSibling != nullptr)
				{
					where = where->nextSibling;
				}
				m_root.nextSibling = nodePtr;
			}
		}

		nodeLookup[handle] = std::move(node);
		return { nodePtr };
	}

	TreeBoxItem TreeBoxReactor::Module::Find(const TreeNodeHandle& handle)
	{
		if (handle.empty())
		{
			return {};
		}

		auto it = nodeLookup.find(handle);
		if (it == nodeLookup.end())
		{
			return {};
		}

		return { it->second.get() };
	}

	TreeNodeHandle TreeBoxReactor::Module::GenerateUniqueHandle(const std::string& key, TreeNodeType* parentNode)
	{
		if (parentNode)
		{
			return parentNode->key + "/" + key;
		}
		return key;
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
