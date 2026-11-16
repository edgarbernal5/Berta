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
#include "Images/ExternalTextureAttributes.h"
#include "Images/SvgImageAttributes.h"

namespace Berta
{
	Image::Image(const std::string& filepath)
	{
		Open(StringUtils::UTF8ToWide(filepath));
	}

	Image::Image(const std::wstring& filepath)
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

	void Image::Open(const std::wstring& filepath)
	{
		std::filesystem::path path{ filepath };
		if (!path.has_extension())
		{
			return;
		}
		
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch){ return std::tolower(ch); });
		
		if (path.extension() == ".svg")
		{
			m_attributes = std::make_shared<SvgImageAttributes>();
		}
		else if (path.extension() == ".ico")
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

	void Image::OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels)
	{
		m_attributes = std::make_shared<BasicImageAttributes>();
		m_attributes->OpenFromMemory(pixels, width, height, channels);
	}

	void Image::UpdateFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels, uint32_t stride)
	{
		if (!m_attributes)
		{
			OpenFromMemory(pixels, width, height, channels);
			return;
		}

		if (auto* basicAttrs = dynamic_cast<BasicImageAttributes*>(m_attributes.get()))
		{
			basicAttrs->UpdateFromMemory(pixels, width, height, channels, stride);
		}
	}

	void Image::UpdateExternalTexture(const ExternalTextureDesc& desc)
	{
		if (!m_attributes)
		{
			m_attributes = std::make_shared<ExternalTextureAttributes>(desc);
			return;
		}

		if (auto* externalAttrs = dynamic_cast<ExternalTextureAttributes*>(m_attributes.get()))
		{
			externalAttrs->Update(desc);
		}
	}

	void Image::AddVariant(float scaleFactor, const std::wstring& filepath)
	{
		m_attributes->AddVariant(scaleFactor, filepath);
	}

	void Image::AddVariantFromMemory(float scaleFactor, const uint8_t* pixels, uint32_t width, uint32_t height, int channels)
	{
		m_attributes->AddVariantFromMemory(scaleFactor, pixels, width, height, channels);
	}

	void Image::Paste(Graphics& destination, const Point& positionDestination) const
	{
		if (!m_attributes)
		{
			return;
		}
		
		Paste(GetLogicalSize().ToRectangle(), destination, positionDestination);
	}

	void Image::Paste(Graphics& destination, const Rectangle& destinationRect) const
	{
		Paste(GetLogicalSize().ToRectangle(), destination, destinationRect);
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

	Image Image::FromExternalTexture(const ExternalTextureDesc& desc)
	{
		Image img;
		img.m_attributes = std::make_shared<ExternalTextureAttributes>(desc);
		return img;
	}
}
