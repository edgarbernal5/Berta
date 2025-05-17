/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include "btpch.h"
#include "Base.h"

#define BERTA_BUILD_ID "v0.1"

namespace Berta
{
	void InitializeCore()
	{
		Log::Initialize();

		BT_CORE_TRACE << "Berta Framework " << BERTA_BUILD_ID << std::endl;
		BT_CORE_TRACE << "Initializing..." << std::endl;
	}

	void ShutdownCore()
	{
		BT_CORE_TRACE << "Shutting down... " << std::endl;

		Log::Shutdown();
	}
}

namespace Berta::StringUtils
{
	std::wstring Convert(const std::string& str)
	{
		/*
		
		if (str.empty()) return std::wstring();

		int tamanoNecesario = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		std::wstring resultado(tamanoNecesario, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &resultado[0], tamanoNecesario);

		// Elimina el caracter nulo final
		resultado.pop_back();
		return resultado;
		*/
		std::wstring wstr(str.begin(), str.end());
		return wstr;
	}

	std::vector<std::string> Split(const std::string& str, char delimiter)
	{
		std::vector<std::string> result;
		std::stringstream ss(str);
		std::string item;
		while (std::getline(ss, item, delimiter))
		{
			if (!item.empty())
			{
				result.push_back(item);
			}
		}
		return result;
	}

	std::vector<std::wstring> Split(const std::wstring& wstr, wchar_t delimiter)
	{
		std::vector<std::wstring> result;
		std::wstringstream ss(wstr);
		std::wstring item;
		while (std::getline(ss, item, delimiter))
		{
			if (!item.empty())
			{
				result.push_back(item);
			}
		}
		return result;
	}
}

namespace Berta::LayoutUtils
{
	float CalculateDPIScaleFactor(uint32_t dpi)
	{
		return static_cast<float>(dpi / BT_APPLICATION_DPI);
	}

	float CalculateDownwardDPIScaleFactor(uint32_t dpi)
	{
		return BT_APPLICATION_DPI / static_cast<float>(dpi);
	}

	bool GetIntersectionClipRect(const Rectangle& parentRectangle, const Rectangle& childRectangle, Rectangle& output)
	{
		if (parentRectangle.X + static_cast<int>(parentRectangle.Width) <= childRectangle.X || childRectangle.X + static_cast<int>(childRectangle.Width) <= parentRectangle.X ||
			parentRectangle.Y + static_cast<int>(parentRectangle.Height) <= childRectangle.Y || childRectangle.Y + static_cast<int>(childRectangle.Height) <= parentRectangle.Y)
		{
			return false;
		}

		// Calculate the intersection rectangle
		int interLeft = (std::max)(parentRectangle.X, childRectangle.X);
		int interTop = (std::max)(parentRectangle.Y, childRectangle.Y);
		int interRight = (std::min)(parentRectangle.X + parentRectangle.Width, childRectangle.X + childRectangle.Width);
		int interBottom = (std::min)(parentRectangle.Y + parentRectangle.Height, childRectangle.Y + childRectangle.Height);

		// Set the intersection rectangle's position and size
		output.X = interLeft;
		output.Y = interTop;
		output.Width = static_cast<uint32_t>(interRight - interLeft);
		output.Height = static_cast<uint32_t>(interBottom - interTop);

		return true;
	}


	bool GetIntersectionClipRect(const Rectangle& sourceRectangle, const Size& sourceSize, const Rectangle& destRectangle, const Size& destSize, Rectangle& outputSourceRect, Rectangle& outputDestRect)
	{
		// Valid clip area for parent and child based on their sizes
		Rectangle validSourceRect{ sourceSize };
		if (!GetIntersectionClipRect(sourceRectangle, validSourceRect, outputSourceRect))
			return false;

		Rectangle validDestRect{ destSize };
		Rectangle resultDestRect;
		if (!GetIntersectionClipRect(destRectangle, validDestRect, resultDestRect))
			return false;

		// Compute proportional offset from original parent rect to output clipped rect
		Scale(sourceRectangle, outputSourceRect, destRectangle, outputDestRect);

		if (Contains(outputDestRect, resultDestRect))
		{
			//GetIntersectionClipRect({ outputChildRect }, resultChildRect, outputChildRect);
		}
		else
		{
			outputDestRect = resultDestRect;

			// Recalculate parent projection from clipped child
			Scale(destRectangle, resultDestRect, sourceRectangle, outputSourceRect);
		}

		return true;
	}

	void Scale(const Rectangle& destScaled, const Rectangle& scaled, const Rectangle& destRect, Rectangle& output)
	{
		double rateX = (double)(scaled.X - destScaled.X) / (double)(destScaled.Width);
		double rateY = (double)(scaled.Y - destScaled.Y) / (double)(destScaled.Height);

		output.X = static_cast<int>(rateX * destRect.Width) + destRect.X;
		output.Y = static_cast<int>(rateY * destRect.Height) + destRect.Y;

		output.Width = static_cast<uint32_t>((double)scaled.Width / (double)(destScaled.Width) * destRect.Width);
		output.Height = static_cast<uint32_t>((double)scaled.Height / (double)(destScaled.Height) * destRect.Height);
	}

	bool Contains(const Rectangle& rect1, const Rectangle& rect2)
	{
		if (rect1.X < rect2.X || rect1.X + rect1.Width > rect2.X + rect2.Width) return false;
		if (rect1.Y < rect2.Y || rect1.Y + rect1.Height > rect2.Y + rect2.Height) return false;

		return true;
	}
}