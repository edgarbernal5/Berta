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
        if (!LayoutUtils::GetIntersectionClipRect(sourceRect, storage.m_size, destinationRect, API::GetPaintHandleSize(destHandle), validSourceDest, validDestRect))
            return;

        ColorBuffer destBuffer;
        destBuffer.Attach(destHandle, validDestRect);

        if (storage.m_hasAlphaChannel)
        {
            //ImageProcessor::ScaleBilinearWithAlphaBlend(*this, validSourceDest, destBuffer, validDestRect);
            ImageProcessor::ScaleNearestAlphaBlend(*this, validSourceDest, destBuffer, validDestRect);
            return;
        }

        ImageProcessor::ScaleNearest(*this, validSourceDest, destBuffer, validDestRect);
    }

    void ColorBuffer::Blend(const Rectangle& sourceRect, PaintNativeHandle* destHandle, const Point& destinationPos, double alpha)
    {
        if (!destHandle || !m_storage)
            return;

        auto& storage = *m_storage.get();
        Rectangle validDestRect, validSourceDest;
        if (!LayoutUtils::GetIntersectionClipRect(sourceRect, storage.m_size, { destinationPos.X, destinationPos.Y, sourceRect.Width, sourceRect.Height },
            API::GetPaintHandleSize(destHandle), validSourceDest, validDestRect))
        {
            return;
        }

        ColorBuffer destBuffer;
        destBuffer.Attach(destHandle, validSourceDest);

        ImageProcessor::AlphaBlend(*this, validSourceDest, destBuffer, { validDestRect.X, validDestRect.Y }, alpha);
    }

    void ColorBuffer::SetAlphaChannel(bool enabled)
    {
        m_storage->m_hasAlphaChannel = enabled;
    }

    ColorABGR& ColorBuffer::Get(size_t index)
    {
        return m_storage->m_buffer[index];
    }

    ColorABGR& ColorBuffer::Get(int x, int y) const
    {
        return *reinterpret_cast<ColorABGR*>(reinterpret_cast<uint8_t*>(m_storage->m_buffer + x) + y * m_storage->m_bytesPerLine);
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
        m_buffer(paintHandle->m_bmpColorBuffer),
        m_size(API::GetPaintHandleSize(paintHandle))
    {
    }

    ColorBuffer::Storage::~Storage()
    {
        if (!m_paintHandle && m_buffer)
        {
            delete[] m_buffer;
            m_buffer = nullptr;
        }
        m_paintHandle = nullptr;
    }

    void ColorBuffer::Storage::Create()
    {
        if (m_size.IsEmpty())
            return;

        m_buffer = new ColorABGR[m_size.Width * m_size.Height];
    }

    void ColorBuffer::Storage::Copy(uint8_t* rawbits, uint32_t width, uint32_t height, uint32_t bitsPerPixel, uint32_t bytesPerLine)
    {
        if (m_size.Width == width && m_size.Height == height && m_bytesPerLine == bytesPerLine && bitsPerPixel == 32)
        {
            memcpy(m_buffer, rawbits, bytesPerLine * m_size.Height);
            return;
        }

        if (bitsPerPixel == 24)
        {

        }
    }
}