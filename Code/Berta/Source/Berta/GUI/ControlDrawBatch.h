/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CONTROL_DRAW_BATCH_HEADER
#define BT_CONTROL_DRAW_BATCH_HEADER

#include "Berta/GUI/Control.h"

namespace Berta
{
	class ControlDrawBatch
	{
	public:
		ControlDrawBatch(ControlBase& control);
		~ControlDrawBatch();

	private:
		ControlBase& m_control;
	};
}

#endif