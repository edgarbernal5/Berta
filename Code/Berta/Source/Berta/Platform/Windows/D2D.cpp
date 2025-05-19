#include "btpch.h"
#include "D2D.h"

namespace Berta::DirectX
{
	D2DModule::D2DModule()
	{
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_handle.m_factory);
	}
}
