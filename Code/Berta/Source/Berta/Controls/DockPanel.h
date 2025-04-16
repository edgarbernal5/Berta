/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_DOCK_PANEL_HEADER
#define BT_DOCK_PANEL_HEADER

#include <string>

#include "Berta/Core/BasicTypes.h"
#include "Berta/GUI/Control.h"

namespace Berta
{
	class ControlBase;

	class DockPanelReactor : public ControlReactor
	{
	public:
		void Update(Graphics& graphics) override;

	};

	class DockPanel : public Control<DockPanelReactor>
	{
	public:
		DockPanel() = default;
		DockPanel(Window* parent, const Rectangle& rectangle = {});
		DockPanel(Window* parent, bool isUnscaleRect, const Rectangle& rectangle = {});
	};

}

#endif