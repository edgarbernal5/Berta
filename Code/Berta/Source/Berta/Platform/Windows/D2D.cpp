#include "btpch.h"
#include "D2D.h"

namespace Berta::DirectX
{
	D2DModule::D2DModule()
	{
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_factory);

		if (FAILED(hr))
		{
			BT_CORE_ERROR << "Error creating D2D factory." << std::endl;
		}

		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_dWriteFactory)
		);

		if (FAILED(hr))
		{
			BT_CORE_ERROR << "Error creating D2D write factory." << std::endl;
		}
	}
}
