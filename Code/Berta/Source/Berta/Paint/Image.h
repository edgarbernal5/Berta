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

		virtual Size GetSize() const = 0;
		virtual void Open(const std::string& filepath) = 0;

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
		
		Image(const Image& other) = default;
		Image(Image&& other) noexcept = default;
		~Image() = default;

		operator bool() const;

		Image& operator=(const Image& rhs) = default;
		Image& operator=(Image&&) noexcept = default;
		bool operator==(const Image& other) const;

		Size GetSize() const { return m_attributes->GetSize(); }

		void Open(const std::string& filepath);
		void Paste(Graphics& destination, const Point& positionDestination) const;
		void Paste(Graphics& destination, const Rectangle& destinationRect) const;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Point& positionDestination) const;
		void Paste(const Rectangle& sourceRect, Graphics& destination, const Rectangle& destinationRect) const;

	private:
		std::shared_ptr<AbstractImageAttributes> m_attributes;
	};
}

#endif