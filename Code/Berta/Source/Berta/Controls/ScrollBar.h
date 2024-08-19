/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_SCROLL_BAR_HEADER
#define BT_SCROLL_BAR_HEADER

#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Core/Timer.h"

#include <string>

namespace Berta
{
	class ScrollBarReactor : public ControlReactor
	{
	public:
		void Init(ControlBase& control) override;
		void Update(Graphics& graphics) override;

		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseDown(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;

		void SetOrientation(bool isVertical);
		void SetMinMax(int min, int max);
		void SetValue(int value);

		int GetMin() const { return m_min; }
		int GetMax() const { return m_max; }
	private:
		enum class InteractionArea
		{
			None,
			Button1,
			Button2,
			Scrollbox,
			ScrollTrack
		};

		bool IsValid() const;
		void DoScrollStep();
		void EmitValueChanged();
		inline bool isScrollable() const { return m_min != m_max; }
		uint32_t GetButtonSize() const;
		void DrawButton(Graphics& graphics, const Rectangle& rect, int arrowLength, int arrowWidth, Graphics::ArrowDirection direction, bool isHighlighted, bool isEnabled);
		InteractionArea DetermineHoverArea(const Point& position) const;
		void UpdateScrollBoxValue(int position, int buttonSize);
		
		Rectangle GetScrollBoxRect() const;

		bool m_isVertical{ false };
		int m_min{ 0 };
		int m_max{ 1 };
		int m_step{ 1 };
		int m_localStep{ 1 };
		int m_pageStep{ 2 };
		int m_value{ 0 };
		Timer m_timer;

		InteractionArea m_hoverArea{ InteractionArea::None };
		InteractionArea m_pressedArea{ InteractionArea::None };
		Point m_mouseDownPosition{};
		int m_prevTrackValue{};
		bool m_trackPageUp{ false };
	};

	class ScrollBar : public Control<ScrollBarReactor, ScrollBarEvents>
	{
	public:
		ScrollBar() = default;
		ScrollBar(Window* parent, const Rectangle& rectangle, bool isVertical = true);
		ScrollBar(Window* parent, bool isUnscaleRect, const Rectangle& rectangle, bool isVertical = true);

		void SetMinMax(int min, int max);
		void SetOrientation(bool isVertical) { m_reactor.SetOrientation(isVertical); }
		void SetValue(int value);

		int GetMin() const { return m_reactor.GetMin(); }
		int GetMax() const { return m_reactor.GetMax(); }
	};
}

#endif
