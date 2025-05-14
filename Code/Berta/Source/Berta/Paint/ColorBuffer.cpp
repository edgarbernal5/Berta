/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ColorBuffer.h"

#include "Berta/API/PaintAPI.h"
#include "Berta/Paint/ImageProcessor.h"
#include "Berta/Core/Base.h"

namespace Berta
{
    void ColorBuffer::Attach(PaintNativeHandle* paintHandle, const Rectangle& targetRect)
    {
        m_storage.reset();
        if (!paintHandle)
            return;

        m_storage = std::make_unique<ColorBuffer::Storage>(paintHandle, targetRect);
    }

    void ColorBuffer::Create(const Size& size)
    {
        m_storage = std::make_unique<ColorBuffer::Storage>(size.Width, size.Height);
    }

    void ColorBuffer::Create(uint32_t width, uint32_t height)
    {
        m_storage = std::make_unique<ColorBuffer::Storage>(width, height);
    }

    void ColorBuffer::Copy(uint8_t* rawbits, uint32_t width, uint32_t height, uint32_t bitsPerPixel, uint32_t bytesPerLine)
    {
        m_storage->Copy(rawbits, width, height, bitsPerPixel, bytesPerLine);
    }

    void ColorBuffer::Paste(const Rectangle& sourceRect, PaintNativeHandle* destHandle, const Rectangle& destinationRect)
    {
        if (!destHandle || !m_storage)
            return;

        auto& storage = *m_storage.get();
        Rectangle validDestRect, validSourceDest;
        auto handleSize = API::GetPaintHandleSize(destHandle);
        if (!LayoutUtils::GetIntersectionClipRect(sourceRect, storage.m_size, destinationRect, handleSize, validSourceDest, validDestRect))
            return;

        if (storage.m_hasAlphaChannel)
        {
            ColorBuffer destBuffer;
            destBuffer.Attach(destHandle, validDestRect);

            //ImageProcessor::ScaleBilinearWithAlphaBlend(*this, validSourceDest, destBuffer, validDestRect);
            ImageProcessor::ScaleNearestAlphaBlend(*this, validSourceDest, destBuffer, validDestRect);
            return;
        }
    }

    void ColorBuffer::Blend(const Rectangle& sourceRect, PaintNativeHandle* destHandle, const Point& destinationPos, double alpha)
    {
        if (!destHandle || !m_storage)
            return;

        auto& storage = *m_storage.get();
        Rectangle validDestRect, validSourceDest;
        auto handleSize = API::GetPaintHandleSize(destHandle);
        if (!LayoutUtils::GetIntersectionClipRect(sourceRect, storage.m_size, { destinationPos.X, destinationPos.Y, sourceRect.Width, sourceRect.Height },
            handleSize, validSourceDest, validDestRect))
        {
            return;
        }

        ColorBuffer destBuffer;
        destBuffer.Attach(destHandle, validSourceDest);

        ImageProcessor::AlphaBlend(*this, validDestRect, destBuffer, {}, alpha);
    }

    void ColorBuffer::SetAlphaChannel(bool enabled)
    {
        m_storage->m_hasAlphaChannel = enabled;
    }

    ColorABGR& ColorBuffer::Get(size_t index)
    {
        return m_storage->m_imageData[index];
    }

    ColorBuffer::Storage::Storage(uint32_t width, uint32_t height) :
        m_size(width, height),
        m_bytesPerLine(width * sizeof(ColorABGR))
    {
        Create();
    }

    ColorBuffer::Storage::Storage(PaintNativeHandle* paintHandle, const Rectangle& targetRect) :
        m_paintHandle(paintHandle),
        m_bytesPerLine(paintHandle->m_bytesPerLine),
        m_imageData(paintHandle->m_bmpColorBuffer),
        m_size(API::GetPaintHandleSize(paintHandle))
    {
    }

    ColorBuffer::Storage::~Storage()
    {
        if (!m_paintHandle && m_imageData)
        {
            delete[] m_imageData;
            m_imageData = nullptr;
        }
        m_paintHandle = nullptr;
    }

    void ColorBuffer::Storage::Create()
    {
        if (m_size.IsEmpty())
            return;

        m_imageData = new ColorABGR[m_size.Width * m_size.Height];
    }

    void ColorBuffer::Storage::Copy(uint8_t* rawbits, uint32_t width, uint32_t height, uint32_t bitsPerPixel, uint32_t bytesPerLine)
    {
        if (m_size.Width == width && m_size.Height == height && m_bytesPerLine == bytesPerLine && bitsPerPixel == 32)
        {
            memcpy(m_imageData, rawbits, bytesPerLine * m_size.Height);
            return;
        }

        if (bitsPerPixel == 24)
        {

        }
    }
}