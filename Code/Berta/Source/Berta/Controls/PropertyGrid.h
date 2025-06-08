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
#include "Berta/Controls/Panel.h"
#include "Berta/Paint/Image.h"

#include <string>
#include <vector>

namespace Berta
{
	struct CategoryItem;

	struct PropertyGridAppearance : public ControlAppearance
	{
		uint32_t CategoryHeight = 22u;
		uint32_t ExpanderButtonSize = 12u;
	};

	struct ArgPropertyGrid
	{
	};

	struct PropertyGridEvents : public ControlEvents
	{
	};

	class FieldControlContainter : public Panel
	{
	public:
		FieldControlContainter() = default;
		FieldControlContainter(Window* parent, const Rectangle& rect = {});
	};

	class PropertyGridField
	{
	public:
		PropertyGridField() = default;
		PropertyGridField(const std::string& label, const std::string value = "") :
			m_label(label), m_value(value), m_defaultValue(value)
		{
		}

		virtual ~PropertyGridField() = default;

		virtual void Create(Window* parent) = 0;

		virtual std::string GetLabel() const;
		virtual void SetLabel(const std::string& label);

		virtual std::string GetValue() const;
		virtual void SetValue(const std::string& value);

		virtual std::string GetDefaultValue() const;
		virtual void SetDefaultValue(const std::string& value);

		virtual uint32_t GetSize() const
		{
			return m_size;
		}

		virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor);

		void Update();

	protected:
		virtual void DrawLabel(Graphics& graphics, const Rectangle& area, const Color& textColor);

	private:
		std::string	m_label;
		std::string	m_value;
		std::string	m_defaultValue;

		uint32_t m_size{ 20 };
	};

	using PropertyGridFieldPtr = std::unique_ptr<PropertyGridField>;

	struct CategoryType
	{
		CategoryType() = default;
		CategoryType(const std::string& name) : m_name(name) {}

		std::string m_name;

		bool m_isExpanded{ true };
		std::vector<std::unique_ptr<PropertyGridField>> m_properties;
		std::vector<std::unique_ptr<FieldControlContainter>> m_fieldContainers;
	};

	class PropertyGridReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		
		class ListModule
		{
		public:
			ListModule();
			CategoryType* CreateCategory(const std::string& categoryName);

			std::vector<CategoryType>::iterator Begin();
			std::vector<CategoryType>::const_iterator Begin() const;
			std::vector<CategoryType>::iterator End();
			std::vector<CategoryType>::const_iterator End() const;

			size_t Size() const
			{
				return m_categories.size();
			}
		private:
			std::vector<CategoryType> m_categories;
		};

		struct ViewportData
		{
			Rectangle m_backgroundRect{};
			bool m_needVerticalScroll{ false };
			uint32_t m_contentSize{};
			uint32_t m_categoryItemHeight{ 0 };
			int m_categoryTextOffset{ 0 };
			uint32_t m_expanderButtonSize{ 0 };
		};

		struct Module
		{
			CategoryItem Append(const std::string& categoryName);
			CategoryItem Find(const std::string& categoryName);
			void Clear();
			void CalculateViewport(ViewportData& viewportData);

			Point m_scrollOffset{};
			ViewportData m_viewport;
			ListModule m_listModule;
			Window* m_owner{ nullptr };
			PropertyGridAppearance* m_appearance{ nullptr };
			std::unique_ptr<ScrollBar> m_scrollBar;
		};

		Module& GetModule() { return m_module; }
		const Module& GetModule() const { return m_module; }

	private:
		Module m_module;
	};

	class PropertyItem
	{
	public:
		PropertyItem(PropertyGridReactor::Module* module, PropertyGridField* propGridField) :
			m_module(module), m_propGridField(propGridField)
		{
		}

		std::string GetLabel() const;
		PropertyItem& SetLabel(const std::string& label);

		std::string GetValue() const;
		PropertyItem& SetValue(const std::string& value, bool emit = false);

	private:
		PropertyGridReactor::Module* m_module{ nullptr };
		PropertyGridField* m_propGridField{ nullptr };
	};

	struct CategoryItem
	{
		CategoryItem() = default;
		CategoryItem(PropertyGridReactor::Module* module, CategoryType* category) : 
			m_module(module), m_category(category)
		{
		}

		PropertyItem Append(PropertyGridFieldPtr propGridField);

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
