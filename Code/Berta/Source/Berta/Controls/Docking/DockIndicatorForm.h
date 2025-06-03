/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_DOCK_INDICATOR_FORM_HEADER
#define BT_DOCK_INDICATOR_FORM_HEADER

#include <string>

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/Control.h"
#include "Berta/GUI/EnumTypes.h"

namespace Berta
{
	enum class DockPosition;

	class DockIndicatorFormReactor : public ControlReactor
	{
	public:
		void Update(Graphics& graphics) override;

		void SetDockPosition(DockPosition position);

	private:
		DockPosition m_dockPosition;
	};

	class DockIndicatorForm : public Control<DockIndicatorFormReactor, FormEvents>
	{
	public:
		explicit DockIndicatorForm(Window* owner, const Size& size, const FormStyle& windowStyle = { true, true, true });
		DockIndicatorForm(Window* owner, const Rectangle& rectangle, const FormStyle& windowStyle = { true, true, true });

		void SetDockPosition(DockPosition position);
	};
}

#endif