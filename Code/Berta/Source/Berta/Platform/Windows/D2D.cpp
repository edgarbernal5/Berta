#include "btpch.h"
#include "D2D.h"

namespace Berta::DirectX
{
	D2DModule::D2DModule()
	{
		D2D1_FACTORY_OPTIONS options = {};
#if BT_DEBUG
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
		options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &options, reinterpret_cast<void**>(&m_factory));

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

	D2DModule::~D2DModule()
	{
		if (m_factory)
		{
			m_factory->Release();
			m_factory = nullptr;
		}

		if (m_dWriteFactory)
		{
			m_dWriteFactory->Release();
			m_dWriteFactory = nullptr;
		}
	}
}
