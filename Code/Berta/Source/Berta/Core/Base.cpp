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
		std::wstring wstr(str.begin(), str.end());
		return wstr;
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
		Rectangle validParentRect{ sourceSize };
		if (!GetIntersectionClipRect(sourceRectangle, validParentRect, outputSourceRect))
			return false;

		Rectangle validChildRect{ destSize };
		Rectangle resultChildRect;
		if (!GetIntersectionClipRect(destRectangle, validChildRect, resultChildRect))
			return false;

		Scale(sourceRectangle, outputSourceRect, destRectangle, outputDestRect);

		if (Contains(outputDestRect, resultChildRect))
		{
			//GetIntersectionClipRect({ outputChildRect }, resultChildRect, outputChildRect);
		}
		else
		{
			outputDestRect = resultChildRect;

			Scale(destRectangle, resultChildRect, sourceRectangle, outputSourceRect);
		}

		return true;
	}

	//bool GetIntersectionClipRect(
	//	const Rectangle& parentRect,
	//	const Size& parentSize,
	//	const Rectangle& childRect,
	//	const Size& childSize,
	//	Rectangle& outParentClip,
	//	Rectangle& outChildClip)
	//{
	//	// Valid clip area for parent and child based on their sizes
	//	Rectangle maxParentClip{ parentSize };
	//	if (!GetIntersectionClipRect(parentRect, maxParentClip, outParentClip))
	//		return false;

	//	Rectangle maxChildClip{ childSize };
	//	Rectangle clippedChild;
	//	if (!GetIntersectionClipRect(childRect, maxChildClip, clippedChild))
	//		return false;

	//	// Compute proportional offset from original parent rect to output clipped rect
	//	auto computeRate = [](int clippedStart, int originalStart, int originalLength) -> double {
	//		return originalLength > 0 ? (clippedStart - originalStart) / static_cast<double>(originalLength) : 0.0;
	//		};

	//	auto scaleLength = [](int clippedLength, int originalLength, int otherLength) -> uint32_t {
	//		return originalLength > 0 ? static_cast<uint32_t>((clippedLength / static_cast<double>(originalLength)) * otherLength) : 0;
	//		};

	//	// Initial child clip projection based on parent clip
	//	double rateX = computeRate(outParentClip.X, parentRect.X, parentRect.Width);
	//	double rateY = computeRate(outParentClip.Y, parentRect.Y, parentRect.Height);

	//	outChildClip.X = static_cast<int>(rateX * childRect.Width) + childRect.X;
	//	outChildClip.Y = static_cast<int>(rateY * childRect.Height) + childRect.Y;
	//	outChildClip.Width = scaleLength(outParentClip.Width, parentRect.Width, childRect.Width);
	//	outChildClip.Height = scaleLength(outParentClip.Height, parentRect.Height, childRect.Height);

	//	if (covered(outChildClip, clippedChild))
	//	{
	//		GetIntersectionClipRect({ outChildClip }, clippedChild, outChildClip);
	//	}
	//	else
	//	{
	//		// Use clipped child rect
	//		outChildClip = clippedChild;

	//		// Recalculate parent projection from clipped child
	//		double reverseRateX = computeRate(clippedChild.X, childRect.X, childRect.Width);
	//		double reverseRateY = computeRate(clippedChild.Y, childRect.Y, childRect.Height);

	//		outParentClip.X = static_cast<int>(reverseRateX * parentRect.Width) + parentRect.X;
	//		outParentClip.Y = static_cast<int>(reverseRateY * parentRect.Height) + parentRect.Y;
	//		outParentClip.Width = scaleLength(clippedChild.Width, childRect.Width, parentRect.Width);
	//		outParentClip.Height = scaleLength(clippedChild.Height, childRect.Height, parentRect.Height);
	//	}

	//	return true;
	//}

	void Scale(const Rectangle& destScaled, const Rectangle& scaled, const Rectangle& destRect, Rectangle& output)
	{
		double rateX = (double)(scaled.X - destScaled.X) / (double)(destScaled.Width);
		double rateY = (double)(scaled.Y - destScaled.Y) / (double)(destScaled.Height);

		output.X = static_cast<int>(rateX * destRect.Width) + destRect.X;
		output.Y = static_cast<int>(rateY * destRect.Height) + destRect.Y;

		output.Width = static_cast<uint32_t>((double)scaled.Width / (double)(destScaled.Width) * destRect.Width);
		output.Height = static_cast<uint32_t>((double)scaled.Height / (double)(destScaled.Height) * destRect.Height);
	}

	bool Contains(const Rectangle& r1, const Rectangle& r2)
	{
		if (r1.X < r2.X || r1.X + r1.Width > r2.X + r2.Width) return false;
		if (r1.Y < r2.Y || r1.Y + r1.Height > r2.Y + r2.Height) return false;

		return true;
	}
}