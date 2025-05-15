/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "ImageProcessor.h"

void Berta::ImageProcessor::ScaleBilinearWithAlphaBlend(ColorBuffer& sourceBuffer, const Rectangle& sourceRect, ColorBuffer& destBuffer, const Rectangle& destRect)
{
    auto bytesPerColor = destBuffer.m_storage->m_bytesPerLine >> 2;

    for (int y = 0; y < destRect.Height; ++y)
    {
        int srcY = destRect.Height == 1 ? 0 : (y * (sourceRect.Height - 1) * 256) / (destRect.Height - 1);
        int y0 = srcY / 256;
        y0 += sourceRect.Y;
        int fy = srcY % 256;
        int y1 = (y0 + 1 < sourceRect.Height) ? y0 + 1 : y0;

        for (int x = 0; x < destRect.Width; ++x)
        {
            int srcX = destRect.Width == 1 ? 0 : (x * (sourceRect.Width - 1) * 256) / (destRect.Width - 1);
            int x0 = srcX / 256;
            x0 += sourceRect.X;
            int fx = srcX % 256;
            int x1 = (x0 + 1 < sourceRect.Width) ? x0 + 1 : x0;

            auto get = [&](int px, int py) -> const ColorABGR&
                {
                    return sourceBuffer.Get(py * sourceRect.Width + px);
                };

            const ColorABGR& c00 = get(x0, y0);
            const ColorABGR& c10 = get(x1, y0);
            const ColorABGR& c01 = get(x0, y1);
            const ColorABGR& c11 = get(x1, y1);

            ColorABGR result;
            for (int k = 0; k < 4; ++k) { // A, B, G, R
                
                int v00 = ((uint8_t*)&c00)[k];
                int v10 = ((uint8_t*)&c10)[k];
                int v01 = ((uint8_t*)&c01)[k];
                int v11 = ((uint8_t*)&c11)[k];

                int top = v00 + ((fx * (v10 - v00)) >> 8);
                int bottom = v01 + ((fx * (v11 - v01)) >> 8);
                int interp = top + ((fy * (bottom - top)) >> 8);

                ((uint8_t*)&result)[k] = (uint8_t)interp;
            }

            ColorABGR& dstPixel = destBuffer.Get((y + destRect.Y) * bytesPerColor + x + destRect.X);

            uint8_t srcA = result.Channels.A;
            uint8_t srcB = result.Channels.B;
            uint8_t srcG = result.Channels.G;
            uint8_t srcR = result.Channels.R;

            uint8_t dstA = dstPixel.Channels.A;
            uint8_t dstB = dstPixel.Channels.B;
            uint8_t dstG = dstPixel.Channels.G;
            uint8_t dstR = dstPixel.Channels.R;

            // Alpha blending (integer math, scaled by 255)
            auto blend = [](uint8_t srcC, uint8_t dstC, uint8_t alpha) -> uint8_t
                {
                    return (uint8_t)(((srcC * alpha) + (dstC * (255 - alpha)) + 127) / 255);
                };

            auto blendA = [](uint8_t srcA, uint8_t dstA) -> uint8_t
                {
                    return (uint8_t)(((srcA * srcA) + (dstA * (255 - srcA)) + 127) / 255);
                };

            dstPixel.Channels.A = blendA(srcA, dstA);
            dstPixel.Channels.B = blend(srcB, dstB, srcA);
            dstPixel.Channels.G = blend(srcG, dstG, srcA);
            dstPixel.Channels.R = blend(srcR, dstR, srcA);
        }
    }
}

void Berta::ImageProcessor::AlphaBlend(ColorBuffer& sourceBuffer, const Rectangle& sourceRect, ColorBuffer& destBuffer, const Point& destPos, double alpha)
{
    auto destBytesPerColor = destBuffer.m_storage->m_bytesPerLine / sizeof(ColorABGR);
    auto sourceBytesPerColor = sourceBuffer.m_storage->m_bytesPerLine / sizeof(ColorABGR);
    uint8_t alphaByte = static_cast<uint8_t>((std::min)(alpha * 255.0, 255.0));

    for (int y = 0; y < sourceRect.Height; ++y)
    {
        for (int x = 0; x < sourceRect.Width; ++x)
        {
            auto get = [&](int px, int py) -> const ColorABGR&
                {
                    return sourceBuffer.Get(py * sourceBytesPerColor + px);
                };

            const ColorABGR& srcPixel = get(x + sourceRect.X, y + sourceRect.Y);
            ColorABGR& dstPixel = destBuffer.Get((y + destPos.Y) * destBytesPerColor + x + destPos.X);

            dstPixel.Channels.A = srcPixel.Channels.A * alpha + dstPixel.Channels.A * (1.0 - alpha);
            dstPixel.Channels.R = srcPixel.Channels.R * alpha + dstPixel.Channels.R * (1.0 - alpha);
            dstPixel.Channels.G = srcPixel.Channels.G * alpha + dstPixel.Channels.G * (1.0 - alpha);
            dstPixel.Channels.B = srcPixel.Channels.B * alpha + dstPixel.Channels.B * (1.0 - alpha);
        }
    }
}

void Berta::ImageProcessor::ScaleNearestAlphaBlend(ColorBuffer& sourceBuffer, const Rectangle& sourceRect, ColorBuffer& destBuffer, const Rectangle& destRect)
{
    auto destBytesPerColor = destBuffer.m_storage->m_bytesPerLine / sizeof(ColorABGR);
    auto sourceBytesPerColor = sourceBuffer.m_storage->m_bytesPerLine / sizeof(ColorABGR);

    for (int y = 0; y < destRect.Height; ++y)
    {
        int srcY = destRect.Height == 1 ? 0 : (y * (sourceRect.Height - 1) * 256) / (destRect.Height - 1);
        int y0 = srcY / 256;
        y0 += sourceRect.Y;

        for (int x = 0; x < destRect.Width; ++x)
        {
            int srcX = destRect.Width == 1 ? 0 : (x * (sourceRect.Width - 1) * 256) / (destRect.Width - 1);
            int x0 = srcX / 256;
            x0 += sourceRect.X;

            auto get = [&](int px, int py) -> const ColorABGR&
                {
                    return sourceBuffer.Get(py * sourceBytesPerColor + px);
                };

            const ColorABGR& result = get(x0, y0);

            ColorABGR& dstPixel = destBuffer.Get((y + destRect.Y) * destBytesPerColor + x + destRect.X);

            uint8_t srcA = result.Channels.A;
            uint8_t srcB = result.Channels.B;
            uint8_t srcG = result.Channels.G;
            uint8_t srcR = result.Channels.R;

            uint8_t dstA = dstPixel.Channels.A;
            uint8_t dstB = dstPixel.Channels.B;
            uint8_t dstG = dstPixel.Channels.G;
            uint8_t dstR = dstPixel.Channels.R;

            // Alpha blending (integer math, scaled by 255)
            auto blend = [](uint8_t srcC, uint8_t dstC, uint8_t alpha) -> uint8_t
                {
                    return (uint8_t)(((srcC * alpha) + (dstC * (255 - alpha)) + 127) / 255);
                };

            auto blendA = [](uint8_t srcA, uint8_t dstA) -> uint8_t
                {
                    return (uint8_t)(((srcA * srcA) + (dstA * (255 - srcA)) + 127) / 255);
                };

            dstPixel.Channels.A = blendA(srcA, dstA);
            dstPixel.Channels.B = blend(srcB, dstB, srcA);
            dstPixel.Channels.G = blend(srcG, dstG, srcA);
            dstPixel.Channels.R = blend(srcR, dstR, srcA);
        }
    }
}

void Berta::ImageProcessor::ScaleNearest(ColorBuffer& sourceBuffer, const Rectangle& sourceRect, ColorBuffer& destBuffer, const Rectangle& destRect)
{
    auto bytesPerColor = destBuffer.m_storage->m_bytesPerLine >> 2;

    for (int y = 0; y < destRect.Height; ++y)
    {
        int srcY = destRect.Height == 1 ? 0 : (y * (sourceRect.Height - 1) * 256) / (destRect.Height - 1);
        int y0 = srcY / 256;
        y0 += sourceRect.Y;

        for (int x = 0; x < destRect.Width; ++x)
        {
            int srcX = destRect.Width == 1 ? 0 : (x * (sourceRect.Width - 1) * 256) / (destRect.Width - 1);
            int x0 = srcX / 256;
            x0 += sourceRect.X;

            auto get = [&](int px, int py) -> const ColorABGR&
                {
                    return sourceBuffer.Get(py * sourceRect.Width + px);
                };

            const ColorABGR& result = get(x0, y0);

            ColorABGR& dstPixel = destBuffer.Get((y + destRect.Y) * bytesPerColor + x + destRect.X);

            dstPixel.Channels.A = result.Channels.A;
            dstPixel.Channels.B = result.Channels.B;
            dstPixel.Channels.G = result.Channels.G;
            dstPixel.Channels.R = result.Channels.R;
        }
    }
}
