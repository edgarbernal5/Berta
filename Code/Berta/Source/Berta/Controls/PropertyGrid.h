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
			friend struct CategoryItem;

		public:
			PropertyGridFieldBase() = default;
			PropertyGridFieldBase(const std::string& label) :
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
			virtual void SetLabel(const std::string& label);

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
			
			virtual void Draw(Graphics& graphics, const Rectangle& area, uint32_t labelWidth, const Color& textColor);

			void EmitEvent();
			void EmitSelectionEvent();
			void ScrollToView();
			void Update();
			
			//TODO
			//Event<> OnValueChanged;
		protected:
			virtual void Create(Window* parent) = 0;
			virtual void DrawLabel(Graphics& graphics, const Rectangle& area, const Color& textColor);
			void SetModule(Module* m_module);

			Window* m_parent{ nullptr };

			std::string	m_label;

			uint32_t m_height{ 24 };
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
			std::unique_ptr<PropertyGridFieldBase> field;
			std::unique_ptr<FieldControlContainer> container;
		};

		struct CategoryType
		{
			StringUtils::StringHash m_id;
			std::string m_name;
			bool m_isExpanded{ true };
			std::vector<PropertyFieldData> m_properties;

			explicit CategoryType(std::string_view name) : m_id(StringUtils::HashString(name)), m_name(name) {}
		};

		class PropertyGridModel
		{
		public:
			PropertyGridModel() = default;
			~PropertyGridModel() = default;

			// Búsqueda y modificación (C++17 string_view)
			CategoryItem AppendCategory(std::string_view categoryName);
			[[nodiscard]] CategoryType* FindCategoryById(StringUtils::StringHash id);
        
			void AppendPropertyToCategory(StringUtils::StringHash catId, std::unique_ptr<PropertyGridFieldBase> field);
			void SetPropertyEnabled(StringUtils::StringHash catId, StringUtils::StringHash propId, bool enabled);
			std::string GetPropertyValueAsString(StringUtils::StringHash catId, StringUtils::StringHash propId);
			void Clear();

			// Getters para la iteración en la Vista
			[[nodiscard]] const std::vector<CategoryType>& GetCategories() const { return m_categories; }
			std::vector<CategoryType>& GetCategories() { return m_categories; }

		private:
			PropertyFieldData* FindPropertyById(StringUtils::StringHash catId, StringUtils::StringHash propId);
			std::vector<CategoryType> m_categories;
		};
		/*struct CategoryType
		{
			CategoryType() = default;
			CategoryType(std::string_view name) : m_name(name) {}

			std::string m_name;
			bool m_isExpanded{ true };
			Rectangle m_area{};
			std::vector<PropertyFieldData> m_items;
		};*/

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

			std::string GetLabel() const;
			PropertyItem& SetLabel(const std::string& label);
			
			/*[[nodiscard]] std::string GetValueAsString() const {
				return m_model ? m_model->GetPropertyValueAsString(m_catId, m_propId) : "";
			}*/
			
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
        
			/*template <typename TControl, typename... Args>
			CategoryItem& EmplaceProperty(Args&&... args)
			{
				if (m_model)
				{
					StringUtils::StringHash propId = StringUtils::HashString(label);
					
					// El modelo busca la categoría de forma segura y le añade la propiedad
					m_model->EmplacePropertyToCategory(m_id, std::make_unique<TControl>(std::forward<Args>(args)...));
				}
				return *this;
			}*/

			// Para usar con el ObjectPool
			PropertyItem AppendProperty(std::unique_ptr<PropertyGridFieldBase> field)
			{
				if (m_model && field) {
					uint32_t propId = StringUtils::HashString(field->GetLabel());
					m_model->AppendPropertyToCategory(m_id, std::move(field));
					return PropertyItem(m_model, m_id, propId);
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
			PropertyGridLayout(Window* owner, const Appearance& config = {});
			~PropertyGridLayout() = default;

			// Recorre el modelo, suma las alturas de lo que está expandido y avisa al scroll
			void CalculateLayout(const PropertyGridModel& model);

			// Dibuja aplicando el offset de m_scrollableView
			void Draw(Graphics& graphics, const PropertyGridModel& model);

			// Expuesto públicamente para que el Reactor enrute eventos (MouseWheel, etc)
			ScrollableView* m_scrollableView{ nullptr };

			// Permite actualizar la configuración visual en caliente
			void SetConfig(const Appearance& config) { m_config = config; }
			[[nodiscard]] const Appearance& GetConfig() const { return m_config; }

		private:
			Window* m_owner{ nullptr };
			std::unique_ptr<ScrollableView> m_internalScrollManager;
			Appearance m_config;
		};
		/*class ListModule
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
		};*/

		struct Module
		{
			CategoryItem Append(std::string_view categoryName);
			CategoryItem Find(std::string_view categoryName);
			
			void Clear();
			void Draw();
			void EmitEvent(PropertyItem item) const;
			void EmitSelectionEvent(PropertyItem item);
			void Update();
			void UpdateScrollBar();
			
			CategoryType* GetCategoryOnMouse(const Point& mousePosition);
			PropertyGridFieldBase* GetCategoryPropertyOnMouse(const Point& mousePosition);
			void ScrollToView(PropertyGridFieldBase* propGridField);

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
