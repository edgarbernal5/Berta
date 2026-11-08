/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "SvgImageAttributes.h"

namespace Berta
{
    SvgImageAttributes::SvgImageAttributes()
    {
    }

    SvgImageAttributes::~SvgImageAttributes()
    {
    }

    Size SvgImageAttributes::GetLogicalSize() const
    {
        return {};
    }

    void SvgImageAttributes::Open(const std::wstring& filepath)
    {
    }

    void SvgImageAttributes::OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels)
    {
    }

    void SvgImageAttributes::AddVariant(float scaleFactor, const std::wstring& filepath)
    {
    }

    void SvgImageAttributes::AddVariantFromMemory(float scaleFactor, const uint8_t* pixels, uint32_t width, uint32_t height, int channels)
    {
    }

    void SvgImageAttributes::Paste(Graphics& destination, const Point& positionDestination)
    {
    }

    void SvgImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect)
    {
    }

    void SvgImageAttributes::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination)
    {
    }
}
