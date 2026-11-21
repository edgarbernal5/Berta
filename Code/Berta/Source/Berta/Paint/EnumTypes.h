/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PAINT_ENUM_TYPES_HEADER
#define BT_PAINT_ENUM_TYPES_HEADER

#include <functional>

namespace Berta
{
	enum class TextureFilterMode : uint8_t
	{
		Linear = 0,
		NearestNeighbor // Inspección de téxeles sin suavizado al hacer zoom
	};

	enum class TextureAlphaMode : uint8_t
	{
		Ignore = 0,     // Recomendado para G-Buffer (evita transparencia si Alpha == 0.0f)
		Premultiplied   // Para texturas con alfa premultiplicado real
	};

	struct ImageDrawOptions
	{
		TextureFilterMode Filter{ TextureFilterMode::Linear };
	};

	struct ExternalTextureDesc
	{
		void* NativeHandle{ nullptr }; // Windows: IDXGISurface* (D3D11On12) | Futuro Vulkan: VkImage/VkImageView
		uint32_t Width{ 0 };
		uint32_t Height{ 0 };
		float DpiScale{ 1.0f };
		TextureAlphaMode AlphaMode{ TextureAlphaMode::Ignore };

		// Hooks de sincronización con el motor (ej. AcquireWrappedResources / ReleaseWrappedResources + Flush)
		std::function<void()> OnBeforeDraw;
		std::function<void()> OnAfterDraw;
	};
}

#endif