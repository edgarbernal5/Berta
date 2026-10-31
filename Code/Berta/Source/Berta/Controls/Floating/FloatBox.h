/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_FLOAT_BOX_HEADER
#define BT_FLOAT_BOX_HEADER

#include <string>
#include "Berta/GUI/Window.h"
#include "Berta/GUI/Control.h"
#include "Berta/Controls/Floating/InteractionData.h"
#include "Berta/Controls/ScrollBar.h"

namespace Berta
{
	class FloatBox;

	namespace Internal::ComboBox
	{
		struct Appearance;
	}

	class FloatBoxReactor : public ControlReactor
	{
	public:
		~FloatBoxReactor() override;

		void Update(Graphics& graphics) override;

		void MouseLeave(Graphics& graphics, const ArgMouse& args) override;
		void MouseMove(Graphics& graphics, const ArgMouse& args) override;
		void MouseUp(Graphics& graphics, const ArgMouse& args) override;
		void MouseWheel(Graphics& graphics, const ArgWheel& args) override;
		void KeyPressed(Graphics& graphics, const ArgKeyboard& args) override;
		void Resize(Graphics& graphics, const ArgResize& args) override;
		
		void SetState(Float::InteractionData& selection);

		struct State
		{
			std::optional<size_t> m_hoveredIndex = std::nullopt;
			std::optional<size_t> m_selectedIndex = std::nullopt;
			int m_offset { 0 };
		};

		State& GetState() {	return m_state; }
		bool MoveSelectedItem(int direction);
		
		void SetItemHeight(uint32_t height) { m_itemHeight = height; }
	protected:
		void DoOnInit() override;
		
	private:
		bool IsInside(const Point& point);
		void UpdateScrollBar();

		FloatBox* m_floatBox{ nullptr };

		Float::InteractionData* m_interactionData{ nullptr };
		uint32_t m_itemHeight{ 24 };
		State m_state;

		bool m_ignoreFirstMouseUp{ true };
		std::unique_ptr<ScrollBar> m_scrollBar;
	};

	class FloatBox : public Control<Category::RootTag, FloatBoxReactor, FormEvents>
	{
	public:
		FloatBox(Window* parent, const Rectangle& rectangle);

		void Init(Float::InteractionData& state, uint32_t itemHeight)
		{
			auto& reactor = GetReactor();
			reactor.SetState(state);
			reactor.SetItemHeight(itemHeight);
		}

		bool MoveSelectedItem(int direction);
		FloatBoxReactor::State& GetState() { return GetReactor().GetState(); }

	private:
	};
}

#endif