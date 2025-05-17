/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_COLOR_BUFFER_HEADER
#define BT_COLOR_BUFFER_HEADER

#include "Berta/Core/BasicTypes.h"
#include <memory>

namespace Berta
{
	struct PaintNativeHandle;

	class ColorBuffer
	{
	public:
		ColorBuffer() = default;

		void Attach(PaintNativeHandle* paintHandle, const Rectangle& targetRect);

		void Create(const Size& size);
		void Create(uint32_t width, uint32_t height);

		void Copy(uint8_t* rawbits, uint32_t width, uint32_t height, uint32_t bitsPerPixel, uint32_t bytesPerLine);

		void Paste(const Rectangle& sourceRect, PaintNativeHandle* destHandle, const Rectangle& destinationRect);
		void Blend(const Rectangle& sourceRect, PaintNativeHandle* destHandle, const Point& destinationPos, double alpha);

		void SetAlphaChannel(bool enabled);

		ColorABGR& Get(size_t index);
		ColorABGR& Get(int x, int y) const;

		struct Storage
		{
			Storage(uint32_t width, uint32_t height);
			Storage(PaintNativeHandle* paintHandle, const Rectangle& targetRect);
			~Storage();

			void Create();
			void Copy(uint8_t* rawbits, uint32_t width, uint32_t height, uint32_t bitsPerPixel, uint32_t bytesPerLine);

			PaintNativeHandle* m_paintHandle{ nullptr };
			ColorABGR* m_buffer{ nullptr };
			Size m_size{};
			uint32_t m_bytesPerLine{ 0 };
			bool m_hasAlphaChannel{ false };
		};

		std::unique_ptr<Storage> m_storage;
	};
}

#endif