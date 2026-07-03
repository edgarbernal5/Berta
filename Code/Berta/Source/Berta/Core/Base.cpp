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
	std::wstring UTF8ToWide(const std::string& utf8Str)
	{
		if (utf8Str.empty())
		{
			return L"";
		}
		int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.size()), nullptr, 0);
		std::wstring wstrTo(sizeNeeded, 0);
	
		MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.size()), wstrTo.data(), sizeNeeded);
		return wstrTo;
	}

	std::string WideToUTF8(const std::wstring& wstr)
	{
		if (wstr.empty())
		{
			return "";
		}
		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
		std::string strTo(sizeNeeded, 0);
		
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), strTo.data(), sizeNeeded, nullptr, nullptr);
		return strTo;
	}

	std::vector<std::string> Split(const std::string& str, char delimiter)
	{
		std::vector<std::string> result;
		size_t start = 0;
		size_t end = str.find(delimiter);

		while (end != std::wstring::npos)
		{
			if (end != start)
			{ 
				result.push_back(str.substr(start, end - start));
			}
			start = end + 1;
			end = str.find(delimiter, start);
		}
    
		if (start < str.length()) 
		{
			result.push_back(str.substr(start));
		}
    
		return result;
	}

	std::vector<std::wstring> Split(const std::wstring& wstr, wchar_t delimiter)
	{
		std::vector<std::wstring> result;
		size_t start = 0;
		size_t end = wstr.find(delimiter);

		while (end != std::wstring::npos)
		{
			if (end != start)
			{ 
				result.push_back(wstr.substr(start, end - start));
			}
			start = end + 1;
			end = wstr.find(delimiter, start);
		}
    
		if (start < wstr.length()) 
		{
			result.push_back(wstr.substr(start));
		}
    
		return result;
	}
}

namespace Berta::LayoutUtils
{
	float CalculateDPIScaleFactor(uint32_t dpi)
	{
		return static_cast<float>(dpi) / BT_APPLICATION_DPI;
	}

	float CalculateDownwardDPIScaleFactor(uint32_t dpi)
	{
		return BT_APPLICATION_DPI / static_cast<float>(dpi);
	}

	bool GetIntersectionRect(const Rectangle& r1, const Rectangle& r2, Rectangle& output)
	{
		if (r1.X + static_cast<int>(r1.Width) <= r2.X || r2.X + static_cast<int>(r2.Width) <= r1.X ||
			r1.Y + static_cast<int>(r1.Height) <= r2.Y || r2.Y + static_cast<int>(r2.Height) <= r1.Y)
		{
			return false;
		}

		// Calculate the intersection rectangle
		int interLeft = std::max<int>(r1.X, r2.X);
		int interTop = std::max<int>(r1.Y, r2.Y);
		int interRight = std::min<int>(r1.X + static_cast<int>(r1.Width), r2.X + static_cast<int>(r2.Width));
		int interBottom = std::min<int>(r1.Y + static_cast<int>(r1.Height), r2.Y + static_cast<int>(r2.Height));

		// Set the intersection rectangle's position and size
		output.X = interLeft;
		output.Y = interTop;
		output.Width = static_cast<uint32_t>(interRight - interLeft);
		output.Height = static_cast<uint32_t>(interBottom - interTop);

		return true;
	}


	bool GetIntersectionRect(const Rectangle& sourceRectangle, const Size& sourceSize, const Rectangle& destRectangle, const Size& destSize, Rectangle& outputSourceRect, Rectangle& outputDestRect)
	{
		// Valid clip area for parent and child based on their sizes
		Rectangle validSourceRect{ sourceSize };
		if (!GetIntersectionRect(sourceRectangle, validSourceRect, outputSourceRect))
		{
			return false;
		}

		Rectangle validDestRect{ destSize };
		Rectangle resultDestRect;
		if (!GetIntersectionRect(destRectangle, validDestRect, resultDestRect))
		{
			return false;
		}

		// Compute proportional offset from original parent rect to output clipped rect
		Scale(sourceRectangle, outputSourceRect, destRectangle, outputDestRect);

		if (Contains(outputDestRect, resultDestRect))
		{
			//GetIntersectionRect({ outputChildRect }, resultChildRect, outputChildRect);
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
		double rateX = static_cast<double>(scaled.X - destScaled.X) / static_cast<double>(destScaled.Width);
		double rateY = static_cast<double>(scaled.Y - destScaled.Y) / static_cast<double>(destScaled.Height);

		output.X = static_cast<int>(rateX * destRect.Width) + destRect.X;
		output.Y = static_cast<int>(rateY * destRect.Height) + destRect.Y;

		output.Width = static_cast<uint32_t>(static_cast<double>(scaled.Width) / static_cast<double>(destScaled.Width) * destRect.Width);
		output.Height = static_cast<uint32_t>(static_cast<double>(scaled.Height) / static_cast<double>(destScaled.Height) * destRect.Height);
	}

	bool Contains(const Rectangle& rect1, const Rectangle& rect2)
	{
		if (rect1.X < rect2.X || rect1.X + rect1.Width > rect2.X + rect2.Width) return false;
		if (rect1.Y < rect2.Y || rect1.Y + rect1.Height > rect2.Y + rect2.Height) return false;

		return true;
	}
}