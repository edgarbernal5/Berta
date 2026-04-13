/*
* MIT License
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PROPERTY_GRID_MODEL_HEADER
#define BT_PROPERTY_GRID_MODEL_HEADER

#include <string>
#include <string_view>
#include <vector>
#include <memory>

// Forward declarations de tus clases base de Berta
namespace Berta::Internal::PropertyGrid
{
    /*class PropertyGridFieldBase;
    
    // Agrupamos el dato para evitar vectores paralelos (Mejora de caché)
    struct PropertyFieldData 
    {
        std::unique_ptr<PropertyGridFieldBase> field;
        // Si necesitas guardar un puntero al contenedor u otra data específica de la propiedad, va aquí.
    };

    struct CategoryType
    {
        std::string m_name;
        bool m_isExpanded{ true };
        std::vector<PropertyFieldData> m_properties;

        explicit CategoryType(std::string_view name) : m_name(name) {}
    };

    class PropertyGridModel
    {
    public:
        PropertyGridModel() = default;
        ~PropertyGridModel() = default;

        // Búsqueda y modificación (C++17 string_view)
        CategoryType& AppendCategory(std::string_view categoryName);
        [[nodiscard]] CategoryType* FindCategory(std::string_view categoryName);
        
        void Clear();

        // Getters para la iteración en la Vista
        [[nodiscard]] const std::vector<CategoryType>& GetCategories() const { return m_categories; }
        std::vector<CategoryType>& GetCategories() { return m_categories; }

    private:
        std::vector<CategoryType> m_categories;
    };*/
}

#endif