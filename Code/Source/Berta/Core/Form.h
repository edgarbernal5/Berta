#ifndef BERTA_FORM_HPP
#define BERTA_FORM_HPP

#include "Widget.h"
#include "BasicTypes.h"

namespace berta
{
	class Form : public Widget
	{
	public:
		Form(const Rectangle& rectangle = { 0,0,800,600 });
	};
}

#endif