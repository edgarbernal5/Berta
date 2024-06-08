/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FONT_PROVIDER_HEADER
#define BT_FONT_PROVIDER_HEADER

namespace Berta
{
	class FontProvider
	{

		//HFONT Graphics::CreateTransparentFont(int height, int weight, bool italic, bool underline)
		//{
		//	::LOGFONT lf;
		//	ZeroMemory(&lf, sizeof(::LOGFONT)); // Clear the structure
		//	lf.lfHeight = height; // Font height
		//	lf.lfWeight = weight; // Font weight (bold)
		//	lf.lfItalic = italic; // Italic style
		//	lf.lfCharSet = DEFAULT_CHARSET; // Character set
		//	lf.lfOutPrecision = OUT_DEFAULT_PRECIS; // Output precision
		//	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS; // Clipping precision
		//	lf.lfQuality = DEFAULT_QUALITY; // Output quality
		//	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE; // Pitch and family
		//	lstrcpy(lf.lfFaceName, L"Segoe UI"); // Font name

		//	// Create the font using CreateFontIndirect
		//	return ::CreateFontIndirect(&lf);
		//}
	};
}

#endif