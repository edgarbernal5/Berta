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
			PropertyGridFieldBase() = default;
			PropertyGridFieldBase(std::string_view label) :
				m_label(label)
			{
			}
			virtual ~PropertyGridFieldBase() = default;
			
			// Prevenir copias accidentales
			PropertyGridFieldBase(const PropertyGridFieldBase&) = delete;
			PropertyGridFieldBase& operator=(const PropertyGridFieldBase&) = delete;
    
			// Permitir move si lo necesitas, o también borrarlo
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
				// Comportamiento base vacío. 
				// Las clases derivadas decidirán qué hacer con este clic.
			}
			
			virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Appearance& config);

			void ScrollToView();
			void Update();
			
			std::function<void()> OnValueChanged;
			std::function<void()> OnSelected;
		protected:
			virtual void Create(Window* parent) = 0;
			virtual void DrawLabel(Graphics& graphics, const Rectangle& area, const Color& textColor);
			
			void NotifyValueChanged();
			void NotifySelected();
			
			Window* m_parent{ nullptr };

			std::string	m_label;

			uint32_t m_height{ 24 };
			bool m_enabled{ true };
		};
		
		class FieldControlContainer : public Panel
		{
		public:
			FieldControlContainer() = default;
			FieldControlContainer(Window* parent, const Rectangle& rect = {});
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
			//std::unique_ptr<FieldControlContainer> container;
			
			// 1. Prohibir Copias explícitamente (= delete)
			PropertyFieldData(const PropertyFieldData&) = delete;
			PropertyFieldData& operator=(const PropertyFieldData&) = delete;

			// 2. Habilitar Movimientos explícitamente (noexcept es vital para std::vector)
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
			
			// 1. Prohibir Copias
			CategoryType(const CategoryType&) = delete;
			CategoryType& operator=(const CategoryType&) = delete;

			// 2. Habilitar Movimientos
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
			// Añadir subcategoría a un padre específico
			CategoryType* AppendSubCategory(StringUtils::StringHash parentId, std::string_view name);
			void AppendPropertyToCategory(StringUtils::StringHash categoryId, std::unique_ptr<PropertyGridFieldBase> field);
        
			// Búsqueda Segura (Recursiva)
			[[nodiscard]] CategoryType* FindCategoryById(StringUtils::StringHash id);
			[[nodiscard]] PropertyFieldData* FindPropertyById(StringUtils::StringHash catId, StringUtils::StringHash propId);
			
			void Clear();

			bool GetPropertyEnabled(StringUtils::StringHash catId, StringUtils::StringHash propId);
			void SetPropertyEnabled(StringUtils::StringHash catId, StringUtils::StringHash propId, bool enabled);
			
			std::string_view GetPropertyLabel(StringUtils::StringHash catId, StringUtils::StringHash propId);
			void SetPropertyLabel(StringUtils::StringHash catId, StringUtils::StringHash propId, std::string_view newLabel);
			
			std::string GetPropertyValueAsString(StringUtils::StringHash catId, StringUtils::StringHash propId);
			
			// Getters para la iteración en la Vista
			[[nodiscard]] const std::vector<CategoryType>& GetRootCategories() const { return m_rootCategories; }
			std::vector<CategoryType>& GetRootCategories() { return m_rootCategories; }
			
			std::function<void()> OnVisualsChanged;
			std::function<void(StringUtils::StringHash catId, StringUtils::StringHash propId)> OnPropertyModified;
			std::function<void(StringUtils::StringHash catId, StringUtils::StringHash propId)> OnPropertySelected;
		private:
			CategoryType* FindRecursive(StringUtils::StringHash id, std::vector<CategoryType>& list);
			void InitCategoryRecursive(CategoryType& cat);
			
			Window* m_ownerWindow{ nullptr };
			std::vector<CategoryType> m_rootCategories;
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
			PropertyItem() = default;
			PropertyItem(PropertyGridModel* model, uint32_t catId, uint32_t propId) :
				m_model(model), m_catId(catId), m_propId(propId)
			{
			}
			
			operator bool() const;

			std::string_view GetLabel() const;
			PropertyItem& SetLabel(std::string_view newLabel);
			
			[[nodiscard]] std::string GetValueAsString() const {
				return m_model ? m_model->GetPropertyValueAsString(m_catId, m_propId) : "";
			}
			
			bool IsEnabled() const;
			PropertyItem& SetEnabled(bool enabled);

		private:
			PropertyGridModel* m_model{ nullptr };
			uint32_t m_catId{ 0 };
			uint32_t m_propId{ 0 };
		};

		struct CategoryItem
		{
			// Constructor vacío crea un handle inválido
			CategoryItem() = default;
        
			CategoryItem(PropertyGridModel* model, uint32_t categoryId)
				: m_model(model), m_id(categoryId) {}

			// --- FLUENT API ---
			// El Handle delega la acción al modelo usando su ID seguro
			CategoryItem AppendCategory(std::string_view name);
			CategoryItem AppendSubCategory(std::string_view name);
			
			template <typename TControl, typename... Args>
			PropertyItem EmplaceProperty(std::string_view label, Args&&... args)
			{
				return AppendProperty(std::make_unique<TControl>(label, std::forward<Args>(args)...));
			}

			PropertyItem AppendProperty(std::unique_ptr<Internal::PropertyGrid::PropertyGridFieldBase> field)
			{
				if (m_model && field)
				{
					uint32_t propId = StringUtils::HashString(field->GetLabel());
					m_model->AppendPropertyToCategory(m_id, std::move(field));
					return {m_model, m_id, propId};
				}
				return {};
			}

			[[nodiscard]] bool IsValid() const { return m_model != nullptr; }

		private:
			PropertyGridModel* m_model{ nullptr };
			uint32_t m_id{ 0 }; // El StringHash FNV-1a de la categoría
		};
		
		class PropertyGridLayout
		{
		public:
			PropertyGridLayout() = default;
			~PropertyGridLayout() = default;

			void Init(Window* owner, const Appearance& config = {});
			// Recorre el modelo, suma las alturas de lo que está expandido y avisa al scroll
			void CalculateLayout(const PropertyGridModel& model);

			// Dibuja aplicando el offset de m_scrollableView
			void Draw(Graphics& graphics, const PropertyGridModel& model, Appearance* appearance);

			// Expuesto públicamente para que el Reactor enrute eventos (MouseWheel, etc)
			ScrollableView* m_scrollableView{ nullptr };

			// Permite actualizar la configuración visual en caliente
			void SetConfig(const Appearance& config) { m_config = config; }
			[[nodiscard]] const Appearance& GetConfig() const { return m_config; }
			
		private:
			uint32_t CalculateCategoryHeight(const CategoryType& cat);
			int DrawRecursive(Graphics& graphics, const CategoryType& cat, int y);
			void DrawCategoryHeader(Graphics& graphics, const Rectangle& area, const CategoryType& cat, const Appearance& config);
			[[nodiscard]] bool IsVisible(const Rectangle& area) const;
			
			Window* m_owner{ nullptr };
			std::unique_ptr<ScrollableView> m_internalScrollManager;
			Appearance m_config;
			bool m_isInitialized { false };
		};

		struct Module
		{
			void Draw();
			void Update();
			
			int ProcessClickRecursive(std::vector<CategoryType>& list, Point pos, int currentY);
			
			PropertyGridModel m_model;
			PropertyGridLayout m_layout;
			Window* m_owner{ nullptr };
			Appearance* m_appearance{ nullptr };

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
			void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
			void Resize(Graphics& graphics, const ArgResize& args) override;

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
	
	using LayoutConfig = Internal::PropertyGrid::Appearance;

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

		CategoryItem Append(std::string_view categoryName);
		void Clear();
		CategoryItem Insert(CategoryItem existingCategory, const std::string& categoryName);
		//CategoryItem Find(std::string_view categoryName);
	};
}

#endif
