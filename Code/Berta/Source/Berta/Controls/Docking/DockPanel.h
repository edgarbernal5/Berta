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
	class DockPanelReactor : public ControlReactor
	{
	public:
		void Update(Graphics& graphics) override;
	};

	class DockPanel : public Control<DockPanelReactor>
	{
	public:
		DockPanel() = default;
		DockPanel(Window* parent, const Rectangle& rectangle = {}, bool visible = true);
		DockPanel(Window* parent, bool isUnscaleRect, const Rectangle& rectangle = {}, bool visible = true);
	};
}

#endif