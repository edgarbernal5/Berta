/*
* MIT License
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "PropertyGridModel.h"

#include <algorithm>

namespace Berta::Internal::PropertyGrid
{
    /*CategoryType& PropertyGridModel::AppendCategory(std::string_view categoryName)
    {
        // Si ya existe, la retornamos
        if (auto* existing = FindCategory(categoryName))
        {
            return *existing;
        }

        // Si no existe, la creamos y la agregamos
        m_categories.emplace_back(categoryName);
        return m_categories.back();
    }

    CategoryType* PropertyGridModel::FindCategory(std::string_view categoryName)
    {
        auto it = std::find_if(m_categories.begin(), m_categories.end(), 
            [categoryName](const CategoryType& cat) { return cat.m_name == categoryName; });

        if (it != m_categories.end())
        {
            return &(*it);
        }
        return nullptr;
    }

    void PropertyGridModel::Clear()
    {
        m_categories.clear();
    }*/
}