/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "RootBuffer.h"

#include "Berta/Paint/Graphics.h"

#include <iostream>
#include <cmath>

namespace Berta
{
	RootBuffer::RootBuffer()
	{
	}

	RootBuffer::RootBuffer(const Size& size, API::NativeWindowHandle nativeHandle)
	{
		Build(size, nativeHandle);
	}

	/*Graphics::Graphics(const Graphics& other)
	{
	}*/

	RootBuffer::RootBuffer(RootBuffer&& other) noexcept
	{
	}

	RootBuffer::~RootBuffer()
	{
		Release();
	}

	RootBuffer& RootBuffer::operator=(const RootBuffer& other)
	{
		if (this != &other)
		{
			/*m_attributes = std::move(other.m_attributes);
			m_dpi = std::move(other.m_dpi);
			m_size = std::move(other.m_size);
			m_renderTarget = std::move(other.m_renderTarget);
			m_bitmapRT = std::move(other.m_bitmapRT);*/
		}
		return *this;
	}

	RootBuffer& RootBuffer::operator=(RootBuffer&& other)
	{
		if (this != &other)
		{
		}

		return *this;
	}

	void RootBuffer::Build(const Size& size, API::NativeWindowHandle nativeWindowHandle)
	{
		
	}

	void RootBuffer::BitBlt(const Rectangle& rectDestination, const Graphics& graphicsSource, const Point& pointSource)
	{
	}

	void RootBuffer::Paste(API::NativeWindowHandle destinationHandle, const Rectangle& areaToUpdate, int x, int y) const
	{
	}

	void RootBuffer::Release()
	{
	}

}