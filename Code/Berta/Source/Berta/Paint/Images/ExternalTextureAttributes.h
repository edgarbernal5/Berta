/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_EXTERNAL_TEXTURE_ATTRIBUTES_HEADER
#define BT_EXTERNAL_TEXTURE_ATTRIBUTES_HEADER

#include <unordered_map>

#if BT_PLATFORM_WINDOWS
#include <wrl/client.h>
#endif

#include <map>

#include "Berta/Paint/Image.h"
#include "Berta/Paint/ColorBuffer.h"

namespace Berta
{
    class ExternalTextureAttributes : public AbstractImageAttributes
    {
    public:
        explicit ExternalTextureAttributes(const ExternalTextureDesc& desc);
        ~ExternalTextureAttributes() override;

        void Update(const ExternalTextureDesc& desc);
        Size GetLogicalSize() const override;

        void Paste(Graphics& destination, const Point& positionDestination, const ImageDrawOptions& options = {}) override;
        void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect, const ImageDrawOptions& options = {}) override;
        void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination, const ImageDrawOptions& options = {}) override;

    private:
        void DrawInternal(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect, const ImageDrawOptions& options);
        void ReleaseNativeObjects();

        ExternalTextureDesc m_desc{};
        Size m_logicalSize{};

#if BT_PLATFORM_WINDOWS
        std::unordered_map<HWND, Microsoft::WRL::ComPtr<ID2D1Bitmap>> m_bitmapCache;
        ID2D1Bitmap* GetOrCreateSharedBitmap(HWND hwnd, ID2D1RenderTarget* renderTarget);
#endif
    };
}

#endif