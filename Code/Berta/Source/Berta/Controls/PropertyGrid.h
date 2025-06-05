/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_HEADER
#define BT_PROPERTY_GRID_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/ScrollBar.h"
#include "Berta/Paint/Image.h"

#include <string>
#include <vector>

namespace Berta
{
	struct CategoryItem;

	struct PropertyGridAppearance : public ControlAppearance
	{
	};

	struct ArgListBox
	{
		size_t SelectedIndex{ 0 };
	};

	struct PropertyGridEvents : public ControlEvents
	{
	};

	class PropertyGridItem
	{
	public:
	};

	struct CategoryType
	{
		CategoryType() = default;
		CategoryType(const std::string& name) : m_name(name) {}

		std::string m_name;

		std::vector<std::unique_ptr<PropertyGridItem>> m_properties;
		bool m_isExpanded{ true };
	};

	class PropertyGridReactor : public ControlReactor
	{
	public:
		void Update(Graphics& graphics) override;
		
		class ListModule
		{
		public:
			ListModule();
			CategoryType* CreateCategory(const std::string& categoryName);

			std::vector<CategoryType>::iterator Begin();
			std::vector<CategoryType>::const_iterator Begin() const;
			std::vector<CategoryType>::iterator End();
			std::vector<CategoryType>::const_iterator End() const;

		private:
			std::vector<CategoryType> m_categories;
		};
		struct Module
		{
			CategoryItem Append(const std::string& categoryName);
			CategoryItem Find(const std::string& categoryName);
			void Clear();

			ListModule m_listModule;
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	struct CategoryItem
	{
		CategoryItem() = default;
		CategoryItem(PropertyGridReactor::Module* module, CategoryType* category) : 
			m_module(module), m_category(category)
		{
		}

		operator bool() const;

		PropertyGridReactor::Module* m_module{ nullptr };
		CategoryType* m_category{ nullptr };
	};

	class PropertyGrid : public Control<PropertyGridReactor, PropertyGridEvents, PropertyGridAppearance>
	{
	public:
		PropertyGrid() = default;
		PropertyGrid(Window* parent, const Rectangle& rectangle = {});

		CategoryItem Append(const std::string& categoryName);
		void Clear();
		CategoryItem Insert(CategoryItem existingCategory, const std::string& categoryName);
	};
}

#endif
