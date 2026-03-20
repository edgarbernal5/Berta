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

	Image::operator bool() const
	{
		return m_attributes.get();
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
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch){ return std::tolower(ch); });
		
		if (path.extension() == ".ico")
		{
			m_attributes = std::make_shared<IconImageAttributes>();
		}
		else if (path.extension() == ".bmp" || path.extension() == ".jpeg" || path.extension() == ".png")
		{
			m_attributes = std::make_shared<BasicImageAttributes>();
		}
		else
		{
			return; 
		}
		
		m_attributes->Open(filepath);
	}

	void Image::Paste(Graphics& destination, const Point& positionDestination) const
	{
		Paste(GetSize().ToRectangle(), destination, positionDestination);
	}

	void Image::Paste(Graphics& destination, const Rectangle& destinationRect) const
	{
		Paste(GetSize().ToRectangle(), destination, destinationRect);
	}

	void Image::Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination) const
	{
		if (!m_attributes)
		{
			return;
		}

		m_attributes->Paste(sourceRect, destination, positionDestination);
	}

	void Image::Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) const
	{
		if (!m_attributes)
		{
			return;
		}

		m_attributes->Paste(sourceRect, destination, destinationRect);
	}
}