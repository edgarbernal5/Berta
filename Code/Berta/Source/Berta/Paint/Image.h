/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_IMAGE_HEADER
#define BT_IMAGE_HEADER

#include "Berta/Core/BasicTypes.h"
#include "Berta/Core/Base.h"
#include "Berta/Paint/ColorBuffer.h"

namespace Berta
{
	class Graphics;

	class AbstractImageAttributes
	{
	public:
		virtual ~AbstractImageAttributes() = default;

		// Devuelve el tamaño lógico (a 96 DPI) para el sistema de layout.
		virtual Size GetLogicalSize() const = 0;
		
		virtual void Open(const std::wstring& filepath) = 0;
		virtual void OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels) = 0;
		
		virtual void AddVariant(float scaleFactor, const std::wstring& filepath) = 0;
		virtual void AddVariantFromMemory(float scaleFactor, const uint8_t* pixels, uint32_t width, uint32_t height, int channels) = 0;
		
		virtual void Paste(Graphics& destination, const Point& positionDestination) = 0;
		virtual void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) = 0;
		virtual void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination) = 0;

	protected:
		ColorBuffer m_colorBuffer;
	};

	class Image
	{
	public:
		Image() = default;
		explicit Image(const std::string& filepath);
		explicit Image(const std::wstring& filepath);
		
		Image(const Image& other) = default;
		Image(Image&& other) noexcept = default;
		~Image() = default;

		operator bool() const;

		Image& operator=(const Image& rhs) = default;
		Image& operator=(Image&&) noexcept = default;
		bool operator==(const Image& other) const;

		Size GetLogicalSize() const { return m_attributes->GetLogicalSize(); }

		void Open(const std::wstring& filepath);
		void OpenFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, int channels);
		
		void AddVariant(float scaleFactor, const std::wstring& filepath);
		void AddVariantFromMemory(float scaleFactor, const uint8_t* pixels, uint32_t width, uint32_t height, int channels);
		
		void Paste(Graphics& destination, const Point& positionDestination) const;
		void Paste(Graphics& destination, const Rectangle& destinationRect) const;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination) const;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) const;

	private:
		std::shared_ptr<AbstractImageAttributes> m_attributes;
	};
}

#endif