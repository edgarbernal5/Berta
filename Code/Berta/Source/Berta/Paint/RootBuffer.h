/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_ROOT_BUFFER_HEADER
#define BT_ROOT_BUFFER_HEADER

#include <memory>
#include "Berta/Core/BasicTypes.h"
#include "Berta/Core/Backend.h"
#include "Berta/API/WindowAPI.h"
#include "Berta/API/PaintAPI.h"

#ifdef BT_PLATFORM_WINDOWS
#include "Berta/Platform/Windows/D2D.h"
#endif

namespace Berta
{
	class Graphics;

	class RootBuffer
	{
	public:
		RootBuffer();
		RootBuffer(const Size& size, API::NativeWindowHandle nativeHandle);
		
		RootBuffer(RootBuffer&& other) noexcept;
		~RootBuffer();
		
		RootBuffer& operator=(const RootBuffer& other);
		RootBuffer& operator=(RootBuffer&& other);

		void Build(const Size& size, API::NativeWindowHandle nativeWindowHandle);
		//void Rebuild(const Size& size);
		void BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource);

		void Paste(API::NativeWindowHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const;
		const Size& GetSize() const { return m_size; }
		
		void Release();

		bool IsValid() const
		{
#ifdef BT_PLATFORM_WINDOWS
			return m_renderTarget;
			//return m_attributes != nullptr && m_attributes->m_hdc;
#else
			return m_attributes != nullptr;
#endif
		}
	private:

#ifdef BT_PLATFORM_WINDOWS
		
		//ID2D1DCRenderTarget* m_renderTarget{ nullptr };
		ID2D1BitmapRenderTarget* m_renderTarget{ nullptr };
#endif
		Size m_size{};
		API::NativeWindowHandle m_nativeWindowHandle;
		std::unique_ptr<PaintNativeHandle> m_attributes;
	};
}

#endif