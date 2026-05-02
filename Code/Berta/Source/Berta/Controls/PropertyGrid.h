/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_HEADER
#define BT_PROPERTY_GRID_HEADER

#include <deque>

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/ScrollBar.h"
#include "Berta/Paint/Image.h"

#include <string>
#include <vector>

#include "Berta/GUI/ScrollableView.h"
#include "Properties/PropertyGridFieldBase.h"

namespace Berta
{
	namespace Internal::PropertyGrid
	{
		constexpr int PG_INDENT_PADDING = 10;
		constexpr int PG_DRAG_THRESHOLD_SQ = 16;
		
		struct Events;
		struct CategoryHandle;
		struct Module;
		
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
			Image m_icon;
			bool m_isExpanded{ true };
			
			std::deque<PropertyFieldData> m_properties;
			std::vector<CategoryType> m_subCategories;
			
			explicit CategoryType(StringUtils::StringHash hashId, std::string_view name, int depth) : 
				m_id(hashId), m_name(name), m_depth(depth)
			{
			}
			
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
			void AppendPropertyToCategory(StringUtils::StringHash categoryId, StringUtils::StringHash propId, std::unique_ptr<PropertyGridFieldBase> field);
        
			[[nodiscard]] const CategoryType* FindCategoryById(StringUtils::StringHash id) const;
			[[nodiscard]] CategoryType* FindCategoryById(StringUtils::StringHash id);
			[[nodiscard]] PropertyFieldData* FindPropertyById(StringUtils::StringHash m_uniqueId) const;
			
			void Clear();
			bool RemoveProperty(StringUtils::StringHash propId);
			bool RemoveCategory(StringUtils::StringHash catId);
			
			bool GetPropertyEnabled(StringUtils::StringHash propId);
			void SetPropertyEnabled(StringUtils::StringHash propId, bool enabled);
			
			std::string_view GetPropertyLabel(StringUtils::StringHash propId);
			void SetPropertyLabel(StringUtils::StringHash propId, std::string_view newLabel);
			
			std::string GetPropertyValueAsString(StringUtils::StringHash propId);
			
			[[nodiscard]] const std::vector<CategoryType>& GetRootCategories() const { return m_rootCategories; }
			std::vector<CategoryType>& GetRootCategories() { return m_rootCategories; }
			
			bool IsCategory(StringUtils::StringHash itemId) const;
			
			void SetSelectedItemId(StringUtils::StringHash id) { m_selectedItemId = id; }
			StringUtils::StringHash GetSelectedItemId() const { return m_selectedItemId; }
			
			bool IsCategoryExpanded(StringUtils::StringHash catId);
			void ToggleCategoryExpansion(StringUtils::StringHash catId);
			
			bool IsShowingCategoryIcons() const { return m_drawImages; }
			void ShowCategoryIcons(bool visible) { m_drawImages = visible; }
			
			void SetCategoryIcon(StringUtils::StringHash catId, const Image& icon);
			
			bool IsRootCategory(StringUtils::StringHash catId);
			void MoveRootCategory(size_t fromIndex, size_t toIndex);
			size_t GetRootCategoryIndex(StringUtils::StringHash catId);
			StringUtils::StringHash GetParentCategory(StringUtils::StringHash propId) const;
			StringUtils::StringHash GetParentId(StringUtils::StringHash childId) const;
			
			std::function<void()> OnVisualsChanged;
			std::function<void(StringUtils::StringHash propId)> OnPropertyModified;
			std::function<void(StringUtils::StringHash propId)> OnPropertySelected;
		private:
			CategoryType* FindRecursive(StringUtils::StringHash id, std::vector<CategoryType>& list);
			const CategoryType* FindRecursive(StringUtils::StringHash id, const  std::vector<CategoryType>& list) const;
			void InitCategoryRecursive(CategoryType& cat);
			bool IsCategoryRecursive(const CategoryType& category, StringUtils::StringHash id) const;
			StringUtils::StringHash GetParentCategoryRecursive(const std::vector<CategoryType> &list, StringUtils::StringHash propId) const;
			StringUtils::StringHash GetParentIdRecursive(const CategoryType& currentCat, StringUtils::StringHash targetId) const;
			bool RemovePropertyRecursive(CategoryType& category, StringUtils::StringHash propId);
			bool RemoveSubCategoryRecursive(CategoryType& parentCat, StringUtils::StringHash targetCatId);
			void CleanUpCategoryLookup(const CategoryType& category);
			
			Window* m_ownerWindow{ nullptr };
			std::unordered_map<StringUtils::StringHash, PropertyFieldData*> m_propertyLookup;
			
			StringUtils::StringHash m_selectedItemId{ 0 };
			std::vector<CategoryType> m_rootCategories;
			bool m_drawImages { false };
		};

		using PropertyGridFieldBasePtr = std::unique_ptr<PropertyGridFieldBase>;

		class PropertyHandle
		{
		public:
			PropertyHandle() = default;
			PropertyHandle(PropertyGridModel* model, StringUtils::StringHash uniqueId) :
				m_model(model), m_uniqueId(uniqueId)
			{
			}
			
			StringUtils::StringHash GetId() const { return m_uniqueId; }
			
			operator bool() const;

			std::string_view GetLabel() const;
			PropertyHandle& SetLabel(std::string_view newLabel);
			
			[[nodiscard]] std::string GetValueAsString() const;
			
			bool IsEnabled() const;
			PropertyHandle& SetEnabled(bool enabled);
			
			template <typename T>
			T* As()
			{
				static_assert(std::is_base_of_v<PropertyGridFieldBase, T>, "T must inherit from PropertyGridFieldBase");
            
				if (!m_model)
				{
					return nullptr;
				}
				
				auto* propData = m_model->FindPropertyById(m_uniqueId);
            
				if (!propData || !propData->field) 
				{
					return nullptr;
				}
				
				return dynamic_cast<T*>(propData->field.get());
			}
			bool operator==(const PropertyHandle& other) const { return m_uniqueId == other.m_uniqueId; }
		private:
			PropertyGridModel* m_model{ nullptr };
			uint32_t m_uniqueId{ 0 };
		};

		struct CategoryHandle
		{
			CategoryHandle() = default;
			CategoryHandle(PropertyGridModel* model, uint32_t categoryId)
				: m_model(model), m_id(categoryId) {}

			StringUtils::StringHash GetId() const { return m_id; }
			
			CategoryHandle AppendCategory(std::string_view name);
			CategoryHandle AppendSubCategory(std::string_view name);
			
			template <typename TControl, typename... Args>
			PropertyHandle EmplaceProperty(std::string_view label, Args&&... args)
			{
				return AppendProperty(m_id, std::make_unique<TControl>(label, std::forward<Args>(args)...));
			}

			PropertyHandle AppendProperty(StringUtils::StringHash catId, std::unique_ptr<PropertyGridFieldBase> field);

			CategoryHandle& SetIcon(const Image& icon);
			
			operator bool() const;
		private:
			PropertyGridModel* m_model{ nullptr };
			StringUtils::StringHash m_id{ 0 };
		};
		
		struct LayoutNodeCache
		{
			StringUtils::StringHash m_id;
			bool m_isCategory;
			Rectangle m_itemRect;
			
			LayoutNodeCache(StringUtils::StringHash id, bool isCategory, const Rectangle& itemRect) :
				m_id(id), m_isCategory(isCategory), m_itemRect(itemRect)
			{}
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
			
			StringUtils::StringHash GetHoveredItemId() const { return m_hoveredItemId; }
			void SetHoverItemId(StringUtils::StringHash itemId) 
			{ 
				m_hoveredItemId = itemId;
			}
			
			const std::vector<LayoutNodeCache>& GetVisibleItemsList() const { return m_visibleItems; }
			
			void RefreshVisibleOnly(const PropertyGridModel& model);
			void ScrollToItem(StringUtils::StringHash targetId);
			Rectangle GetItemRect(StringUtils::StringHash id) const;
			
			void SetDropIndicator(bool show, size_t targetIndex = 0);
		private:
			uint32_t CalculateRecursive(const CategoryType& cat, int currentX, uint32_t currentY);
			
			void DrawCategoryHeader(Graphics& graphics, const PropertyGridModel& model, const Rectangle& area, const CategoryType& cat, Appearance* config);
			
			void SyncControlsVisibility(const PropertyGridModel& model);
			void HideAllControlsRecursive(const CategoryType& cat) const;
			
			Window* m_owner{ nullptr };
			std::unique_ptr<ScrollableView> m_internalScrollManager;
			
			StringUtils::StringHash m_hoveredItemId { 0 };
			Appearance* m_config;
			std::unordered_map<StringUtils::StringHash, Rectangle> m_itemRects;
			std::vector<LayoutNodeCache> m_visibleItems;
			bool m_isInitialized { false };
			bool m_showDropIndicator = false;
			size_t m_dropIndicatorIndex = 0;
		};

		struct Module
		{
			struct HitResult 
			{
				StringUtils::StringHash id = 0;
				bool isCategory = false;
    
				operator bool() const { return id != 0; }
			};
			void Draw();
			void Update();
			
			void ClearReferences(StringUtils::StringHash deletedId);
			void OnLayoutChanged();
			HitResult HitTest(Point mousePos) const;
			
			PropertyGridModel m_model;
			PropertyGridLayout m_layout;
			StringUtils::StringHash m_lastHoveredItemId { 0 };
			StringUtils::StringHash m_pressedItemId{ 0 };
			Point m_mouseDownPos{0, 0};
			bool m_isWaitingForDrag = false;
			bool m_isDraggingCategory = false;
			size_t m_draggedCatIndex = 0;
			size_t m_hoveredDropIndex = 0;
			
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
			void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
			void Resize(Graphics& graphics, const ArgResize& args) override;
			void DpiChanged(Graphics& graphics) override;

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
		Internal::PropertyGrid::PropertyHandle Property;
		explicit ArgPropertyGrid(const Internal::PropertyGrid::PropertyHandle& item) : Property(item) {}
	};
	
	struct ArgPropertyGridCategory
	{
		Internal::PropertyGrid::CategoryHandle Category;
		explicit ArgPropertyGridCategory(const Internal::PropertyGrid::CategoryHandle& item) : Category(item) {}
	};

	namespace Internal::PropertyGrid
	{
		struct Events : public ControlEvents
		{
			Event<ArgPropertyGrid> PropertyChanged;
			Event<ArgPropertyGrid> SelectionChanged;
			
			Event<ArgPropertyGrid> PropertyRightClicked;
			Event<ArgPropertyGrid> PropertyDoubleClicked;
			
			Event<ArgPropertyGridCategory> CategoryClicked;
			Event<ArgPropertyGridCategory> CategoryRightClicked;
		};
	}

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
		
		void Erase(CategoryItem categoryItem);
		void Erase(PropertyItem propertyItem);
		
		void RefreshAll();
		void ShowCategoryIcons(bool visible);
	};
}

#endif
