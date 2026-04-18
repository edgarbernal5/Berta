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

#include "Berta/GUI/ScrollableView.h"

namespace Berta
{
	namespace Internal::PropertyGrid
	{
		struct Events;
		struct CategoryHandle;
		struct Module;

		struct Appearance : public ControlAppearance
		{
			uint32_t CategoryHeight = 22u;
			uint32_t ExpanderButtonSize = 12u;
			
			Color HoverBackgroundColor{ 60, 60, 60, 255 };
			Color SelectedBackgroundColor{ 0, 112, 192, 255 };
			Color SelectedTextColor{ 255, 255, 255, 255 };
			Color NormalTextColor{ 200, 200, 200, 255 };
		};
		
		class PropertyGridFieldBase
		{
		public:
			PropertyGridFieldBase() = default;
			PropertyGridFieldBase(std::string_view label) :
				m_label(label)
			{
			}
			virtual ~PropertyGridFieldBase() = default;
			
			PropertyGridFieldBase(const PropertyGridFieldBase&) = delete;
			PropertyGridFieldBase& operator=(const PropertyGridFieldBase&) = delete;
			
			PropertyGridFieldBase(PropertyGridFieldBase&&) = default;
			PropertyGridFieldBase& operator=(PropertyGridFieldBase&&) = default;

			void Init(Window* parent);

			virtual std::string_view GetLabel() const;
			virtual void SetLabel(std::string_view newLabel);

			[[nodiscard]] virtual std::string GetValueAsString() const = 0;
			
			virtual bool IsEnabled() const;
			virtual void SetEnabled(bool enabled);

			virtual uint32_t GetHeight() const
			{
				return m_parent->ToScale(m_height);
			}
			
			virtual void OnMouseClick(const Point& localPosition, uint32_t labelWidth) 
			{
			}
			
			virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Appearance& config);

			void ScrollToView();
			void Update();
			
			void SetVisibility(bool visible);
			
			std::function<void()> OnValueChanged;
			std::function<void()> OnSelected;
			
		protected:
			virtual void Create(Window* parent) = 0;
			virtual void DrawLabel(Graphics& graphics, const Rectangle& area, const Color& textColor);
			virtual void OnVisibilityChanged(bool visible) {}
			
			void NotifyValueChanged();
			void NotifySelected();
			
			Window* m_parent{ nullptr };

			std::string	m_label;

			uint32_t m_height{ 24 };
			bool m_enabled{ true };
			bool m_isVisible{ true };
		};
		
		/*
		 *TODO
		using FieldCreator = std::function<std::unique_ptr<PropertyGridFieldBase>()>;
		std::unordered_map<std::type_index, FieldCreator> m_fieldRegistry;
		public:
		template<typename T, typename FieldType>
		void RegisterField() {
			m_fieldRegistry[typeid(T)] = []() { return std::make_unique<FieldType>(); };
		}
		// En la inicialización de Berta:
		grid.RegisterField<std::string, PropertyGridFieldString>();
		// grid.RegisterField<int, PropertyGridFieldInt>();
		*/
		
		struct PropertyFieldData
		{
			StringUtils::StringHash m_id;
			
			std::unique_ptr<PropertyGridFieldBase> field;
			
			PropertyFieldData(const PropertyFieldData&) = delete;
			PropertyFieldData& operator=(const PropertyFieldData&) = delete;
			
			PropertyFieldData(PropertyFieldData&&) noexcept = default;
			PropertyFieldData& operator=(PropertyFieldData&&) noexcept = default;
		};

		struct CategoryType
		{
			StringUtils::StringHash m_id;
			std::string m_name;
			uint32_t m_depth{ 0 };
			bool m_isExpanded{ true };
			
			std::vector<PropertyFieldData> m_properties;
			std::vector<CategoryType> m_subCategories;
			
			explicit CategoryType(std::string_view name, int depth) : 
				m_id(StringUtils::HashString(name)), m_name(name), m_depth(depth) {}
			
			CategoryType(const CategoryType&) = delete;
			CategoryType& operator=(const CategoryType&) = delete;

			CategoryType(CategoryType&&) noexcept = default;
			CategoryType& operator=(CategoryType&&) noexcept = default;
		};

		class PropertyGridModel
		{
		public:
			PropertyGridModel() = default;
			~PropertyGridModel() = default;
			
			void Init(Window* ownerWindow);
			
			CategoryType& AppendRootCategory(std::string_view categoryName);
			CategoryType* AppendSubCategory(StringUtils::StringHash parentId, std::string_view name);
			void AppendPropertyToCategory(StringUtils::StringHash categoryId, std::unique_ptr<PropertyGridFieldBase> field);
        
			[[nodiscard]] CategoryType* FindCategoryById(StringUtils::StringHash id);
			[[nodiscard]] PropertyFieldData* FindPropertyById(StringUtils::StringHash catId, StringUtils::StringHash propId);
			
			void Clear();

			bool GetPropertyEnabled(StringUtils::StringHash catId, StringUtils::StringHash propId);
			void SetPropertyEnabled(StringUtils::StringHash catId, StringUtils::StringHash propId, bool enabled);
			
			std::string_view GetPropertyLabel(StringUtils::StringHash catId, StringUtils::StringHash propId);
			void SetPropertyLabel(StringUtils::StringHash catId, StringUtils::StringHash propId, std::string_view newLabel);
			
			std::string GetPropertyValueAsString(StringUtils::StringHash catId, StringUtils::StringHash propId);
			
			[[nodiscard]] const std::vector<CategoryType>& GetRootCategories() const { return m_rootCategories; }
			std::vector<CategoryType>& GetRootCategories() { return m_rootCategories; }
			
			void SetSelectedProperty(StringUtils::StringHash catId, StringUtils::StringHash propId);
			
			[[nodiscard]] StringUtils::StringHash GetSelectedCatId() const { return m_selectedCatId; }
			[[nodiscard]] StringUtils::StringHash GetSelectedPropId() const { return m_selectedPropId; }
			
			std::function<void()> OnVisualsChanged;
			std::function<void(StringUtils::StringHash catId, StringUtils::StringHash propId)> OnPropertyModified;
			std::function<void(StringUtils::StringHash catId, StringUtils::StringHash propId)> OnPropertySelected;
		private:
			CategoryType* FindRecursive(StringUtils::StringHash id, std::vector<CategoryType>& list);
			void InitCategoryRecursive(CategoryType& cat);

			Window* m_ownerWindow{ nullptr };
			StringUtils::StringHash m_selectedCatId{ 0 };
			StringUtils::StringHash m_selectedPropId{ 0 };
			std::vector<CategoryType> m_rootCategories;
		};

		using PropertyGridFieldBasePtr = std::unique_ptr<PropertyGridFieldBase>;

		class PropertyHandle
		{
		public:
			PropertyHandle() = default;
			PropertyHandle(PropertyGridModel* model, uint32_t catId, uint32_t propId) :
				m_model(model), m_catId(catId), m_propId(propId)
			{
			}
			
			operator bool() const;

			std::string_view GetLabel() const;
			PropertyHandle& SetLabel(std::string_view newLabel);
			
			[[nodiscard]] std::string GetValueAsString() const;
			
			bool IsEnabled() const;
			PropertyHandle& SetEnabled(bool enabled);

		private:
			PropertyGridModel* m_model{ nullptr };
			uint32_t m_catId{ 0 };
			uint32_t m_propId{ 0 };
		};

		struct CategoryHandle
		{
			CategoryHandle() = default;
			CategoryHandle(PropertyGridModel* model, uint32_t categoryId)
				: m_model(model), m_id(categoryId) {}

			CategoryHandle AppendCategory(std::string_view name);
			CategoryHandle AppendSubCategory(std::string_view name);
			
			template <typename TControl, typename... Args>
			PropertyHandle EmplaceProperty(std::string_view label, Args&&... args)
			{
				return AppendProperty(std::make_unique<TControl>(label, std::forward<Args>(args)...));
			}

			PropertyHandle AppendProperty(std::unique_ptr<PropertyGridFieldBase> field);

			operator bool() const;
		private:
			PropertyGridModel* m_model{ nullptr };
			uint32_t m_id{ 0 };
		};
		
		class PropertyGridLayout
		{
		public:
			PropertyGridLayout() = default;
			~PropertyGridLayout() = default;

			void Init(Window* owner, Appearance* config);
			void CalculateLayout(const PropertyGridModel& model);

			void Draw(Graphics& graphics, const PropertyGridModel& model, Appearance* appearance);

			ScrollableView* m_scrollableView{ nullptr };

			void SetConfig(Appearance* config) { m_config = config; }
			[[nodiscard]] Appearance* GetConfig() const { return m_config; }
			
			void SetHoverState(StringUtils::StringHash catId, StringUtils::StringHash propId) 
			{ 
				m_hoveredCatId = catId; 
				m_hoveredPropId = propId; 
			}
			
		private:
			uint32_t CalculateCategoryHeight(const CategoryType& cat);
			
			int DrawRecursive(Graphics& graphics, const PropertyGridModel& model, const CategoryType& cat, int y);
			void DrawCategoryHeader(Graphics& graphics, const Rectangle& area, const CategoryType& cat, Appearance* config);
			
			[[nodiscard]] bool IsVisible(const Rectangle& area) const;
			void HideCategoryRecursive(const CategoryType& cat);
			
			Window* m_owner{ nullptr };
			std::unique_ptr<ScrollableView> m_internalScrollManager;
			
			StringUtils::StringHash m_hoveredCatId { 0 };
			StringUtils::StringHash m_hoveredPropId { 0 };
			Appearance* m_config;
			
			bool m_isInitialized { false };
		};

		struct Module
		{
			void Draw();
			void Update();
			
			int ProcessClickRecursive(std::vector<CategoryType>& list, Point pos, int currentY, Appearance* appearance);
			int HitTestRecursive(const std::vector<CategoryType>& list, Point pos, int currentY, StringUtils::StringHash& outCatId, StringUtils::StringHash& outPropId);
			
			PropertyGridModel m_model;
			PropertyGridLayout m_layout;
			StringUtils::StringHash m_lastHoveredCat { 0 };
			StringUtils::StringHash m_lastHoveredProp { 0 };
			
			Window* m_owner{ nullptr };

			Events* m_events{ nullptr };
			Graphics* m_graphics{ nullptr };
		};

		class Reactor : public ControlReactor
		{
		public:
			void Update(Graphics& graphics) override;
			void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
			void MouseDown(Graphics& graphics, const ArgMouse& args) override;
			void MouseMove(Graphics& graphics, const ArgMouse& args) override;
			void MouseUp(Graphics& graphics, const ArgMouse& args) override;
			void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
			void Resize(Graphics& graphics, const ArgResize& args) override;
			void DpiChanged(Graphics& graphics) override;

			Module& GetModule() { return m_module; }
			const Module& GetModule() const { return m_module; }

		protected:
			void DoOnInit() override;
			void OnLayoutChanged();
			
		private:
			Module m_module;
		};
	};	

	struct ArgPropertyGrid
	{
		Internal::PropertyGrid::PropertyHandle Property;
		ArgPropertyGrid(const Internal::PropertyGrid::PropertyHandle& item) : Property(item) {}
	};

	namespace Internal::PropertyGrid
	{
		struct Events : public ControlEvents
		{
			Event<ArgPropertyGrid> PropertyChanged;
			Event<ArgPropertyGrid> SelectionChanged;
		};
	}
	
	using LayoutConfig = Internal::PropertyGrid::Appearance;

	class PropertyGrid : public Control<Category::ControlTag, Internal::PropertyGrid::Reactor, Internal::PropertyGrid::Events, Internal::PropertyGrid::Appearance>
	{
	public:
		using CategoryItem = Internal::PropertyGrid::CategoryHandle;
		using PropertyItem = Internal::PropertyGrid::PropertyHandle;
		using PropertyGridFieldBase = Internal::PropertyGrid::PropertyGridFieldBase;
		using PropertyGridFieldBasePtr = Internal::PropertyGrid::PropertyGridFieldBasePtr;

	public:
		PropertyGrid() = default;
		PropertyGrid(Window* parent, const Rectangle& rectangle = {});

		CategoryItem Append(std::string_view categoryName);
		void Clear();
		CategoryItem Insert(CategoryItem existingCategory, const std::string& categoryName);
		//CategoryItem Find(std::string_view categoryName);
	};
}

#endif
