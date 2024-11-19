/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Image.h"

#include "Berta/Paint/Graphics.h"
#include "Berta/Paint/Images/IconImageAttributes.h"
#include "Berta/Paint/Images/BasicImageAttributes.h"

namespace Berta
{
	Image::Image(const std::string& filepath)
	{
		Open(filepath);
	}

	Image::Image(const Image& other) : 
		m_attributes(other.m_attributes)
	{
	}

	Image::Image(Image&& other) noexcept :
		m_attributes(std::move(other.m_attributes))
	{
	}

	Image::~Image()
	{
		if (m_attributes)
		{
			m_attributes.reset();
		}
	}

	Image::operator bool() const
	{
		return m_attributes.get();
	}

	Image& Image::operator=(const Image& rhs)
	{
		if (this != &rhs)
		{
			m_attributes = rhs.m_attributes;
		}

		return *this;
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		if (this != &other)
		{
			m_attributes = std::move(other.m_attributes);
		}

		return *this;
	}

	bool Image::operator==(const Image& other) const
	{
		return m_attributes == other.m_attributes;
	}

	void Image::Open(const std::string& filepath)
	{
		std::filesystem::path path{ filepath };
		if (!path.has_extension())
		{
			return;
		}

		if (path.extension() == ".ico")
		{
			m_attributes = std::make_shared<IconImageAttributes>();
		}
		else if (path.extension() == ".bmp" || path.extension() == ".jpeg" || path.extension() == ".png")
		{
			m_attributes = std::make_shared<BasicImageAttributes>();
		}

		m_attributes->Open(filepath);
	}

	void Image::Paste(Graphics& destination, const Point& positionDestination)
	{
		Paste(Rectangle{ GetSize() }, destination, positionDestination);
	}

	void Image::Paste(Graphics& destination, const Rectangle& destinationRect)
	{
		Paste(Rectangle{ GetSize() }, destination, destinationRect);
	}

	void Image::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination)
	{
		if (!m_attributes)
		{
			return;
		}

		m_attributes->Paste(destination, positionDestination);
	}

	void Image::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect)
	{
		if (!m_attributes)
		{
			return;
		}

		m_attributes->Paste(sourceRect, destination, destinationRect);
	}
}