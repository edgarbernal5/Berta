/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_CONTROL_WINDOW_HEADER
#define BT_CONTROL_WINDOW_HEADER

namespace Berta
{
	class ControlBase;

	class ControlWindowInterface
	{
	public:
		virtual ~ControlWindowInterface() = default;

		virtual ControlBase* ControlPtr() const = 0;
		virtual void Destroy() = 0;
	};
}

#endif