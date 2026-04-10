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
	namespace Internal::PropertyGrid
	{
		struct Events;
		struct CategoryItem;
		struct Module;

		struct Appearance : public ControlAppearance
		{
			uint32_t CategoryHeight = 22u;
			uint32_t ExpanderButtonSize = 12u;
		};

		class PropertyGridFieldBase
		{
		public:
			friend struct CategoryItem;

		public:
			PropertyGridFieldBase() = default;
			PropertyGridFieldBase(const std::string& label, const std::string& value = "") :
				m_label(label), m_value(value), m_defaultValue(value)
			{
			}

			virtual ~PropertyGridFieldBase() = default;

			void Init(Window* parent);

			virtual std::string GetLabel() const;
			virtual void SetLabel(const std::string& label);

			virtual std::string GetValue() const;
			virtual void SetValue(const std::string& value);

			virtual std::string GetDefaultValue() const;
			virtual void SetDefaultValue(const std::string& value);

			virtual bool IsEnabled() const;
			virtual void SetEnabled(bool enabled);

			virtual uint32_t GetSize() const
			{
				return m_parent->ToScale(m_size);
			}

			virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor);

			void EmitEvent();
			void EmitSelectionEvent();
			void ScrollToView();
			void Update();

		protected:
			virtual void Create(Window* parent) = 0;
			virtual void DrawLabel(Graphics& graphics, const Rectangle& area, const Color& textColor);
			void SetModule(Module* m_module);

			Window* m_parent{ nullptr };

			std::string	m_label;
			std::string	m_value;
			std::string	m_defaultValue;

			uint32_t m_size{ 24 };
			bool m_enabled{ true };

		private:
			Module* m_module{ nullptr };
		};

		class FieldControlContainer : public Panel
		{
		public:
			FieldControlContainer() = default;
			FieldControlContainer(Window* parent, const Rectangle& rect = {});
		};

		struct CategoryType
		{
			CategoryType() = default;
			CategoryType(const std::string& name) : m_name(name) {}

			std::string m_name;

			bool m_isExpanded{ true };
			Rectangle m_area{};
			std::vector<std::unique_ptr<PropertyGridFieldBase>> m_properties;
			std::vector<std::unique_ptr<FieldControlContainer>> m_fieldContainers;
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

		struct MouseInteraction
		{
			CategoryType* m_hoveredCategory{ nullptr };
			CategoryType* m_selectedCategory{ nullptr };
			PropertyGridFieldBase* m_lastPropertySelected{ nullptr };
		};

		using PropertyGridFieldBasePtr = std::unique_ptr<PropertyGridFieldBase>;

		class PropertyItem
		{
		public:
			friend struct Module;

		public:
			PropertyItem() = default;
			PropertyItem(Module* module, PropertyGridFieldBase* propGridField) :
				m_module(module), m_propGridField(propGridField)
			{
			}
			
			operator bool() const;

			std::string GetLabel() const;
			PropertyItem& SetLabel(const std::string& label);

			std::string GetValue() const;
			PropertyItem& SetValue(const std::string& value, bool emitEvent = false);

			bool IsEnabled() const;
			PropertyItem& SetEnabled(bool enabled);
			
			PropertyGridFieldBase* GetPropertyFieldPtr() const { return m_propGridField; }

		private:
			Module* m_module{ nullptr };
			PropertyGridFieldBase* m_propGridField{ nullptr };
		};

		struct CategoryItem
		{
			CategoryItem() = default;
			CategoryItem(Module* module, CategoryType* category) :
				m_module(module), m_category(category)
			{
			}

			PropertyItem Append(PropertyGridFieldBasePtr propGridFieldPtr);

			operator bool() const;

			Module* m_module{ nullptr };
			CategoryType* m_category{ nullptr };
		};

		class ListModule
		{
		public:
			ListModule() = default;
			CategoryType* CreateCategory(const std::string& categoryName);

			std::vector<CategoryType>::iterator Begin();
			std::vector<CategoryType>::const_iterator Begin() const;
			std::vector<CategoryType>::iterator End();
			std::vector<CategoryType>::const_iterator End() const;

			void Clear();
			
			size_t Size() const
			{
				return m_categories.size();
			}
		private:
			std::vector<CategoryType> m_categories;
		};

		struct Module
		{
			CategoryItem Append(const std::string& categoryName);
			void BuildItems();
			CategoryItem Find(const std::string& categoryName);
			void Clear();
			void CalculateViewport(ViewportData& viewportData);
			void CalculateContentSize(ViewportData& viewportData);
			void Draw();
			void EmitEvent(PropertyItem item) const;
			void EmitSelectionEvent(PropertyItem item);
			void Update();
			void UpdateScrollBar();
			
			CategoryType* GetCategoryOnMouse(const Point& mousePosition);
			PropertyGridFieldBase* GetCategoryPropertyOnMouse(const Point& mousePosition);
			void ScrollToView(PropertyGridFieldBase* propGridField);

			Point m_scrollOffset{};
			ViewportData m_viewport;
			ListModule m_listModule;
			Window* m_owner{ nullptr };
			Appearance* m_appearance{ nullptr };
			std::unique_ptr<ScrollBar> m_scrollBar;

			Events* m_events{ nullptr };
			Graphics* m_graphics{ nullptr };
			MouseInteraction m_mouseInteraction;
		};

		class Reactor : public ControlReactor
		{
		public:
			void Update(Graphics& graphics) override;
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void MouseMove(Graphics& graphics, const ArgMouse& args) override;
			void MouseUp(Graphics& graphics, const ArgMouse& args) override;
			void Resize(Graphics& graphics, const ArgResize& args) override;

			Module& GetModule() { return m_module; }
			const Module& GetModule() const { return m_module; }

		protected:
			void DoOnInit() override;
			
		private:
			Module m_module;
		};
	};	

	struct ArgPropertyGrid
	{
		Internal::PropertyGrid::PropertyItem Property;
		ArgPropertyGrid(const Internal::PropertyGrid::PropertyItem& item) : Property(item) {}
	};

	namespace Internal::PropertyGrid
	{
		struct Events : public ControlEvents
		{
			Event<ArgPropertyGrid> PropertyChanged;
			Event<ArgPropertyGrid> SelectionChanged;
		};
	}

	class PropertyGrid : public Control<Category::ControlTag, Internal::PropertyGrid::Reactor, Internal::PropertyGrid::Events, Internal::PropertyGrid::Appearance>
	{
	public:
		using CategoryItem = Internal::PropertyGrid::CategoryItem;
		using PropertyItem = Internal::PropertyGrid::PropertyItem;
		using PropertyGridFieldBase = Internal::PropertyGrid::PropertyGridFieldBase;
		using PropertyGridFieldBasePtr = Internal::PropertyGrid::PropertyGridFieldBasePtr;

	public:
		PropertyGrid() = default;
		PropertyGrid(Window* parent, const Rectangle& rectangle = {});

		CategoryItem Append(const std::string& categoryName);
		void Clear();
		CategoryItem Insert(CategoryItem existingCategory, const std::string& categoryName);
		CategoryItem Find(const std::string& categoryName);
	};
}

#endif
