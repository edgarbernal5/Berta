/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <D3D12Lite.h>

int main()
{
	std::unique_ptr<D3D12Lite::Device> device;
	std::unique_ptr<D3D12Lite::GraphicsContext> graphicsContext;

	Berta::Form form(Berta::Size(450u, 350u), { true, true, true }, true);
	form.SetCaption("D3D - Example");
	form.SetCustomPaintCallback([&device, &graphicsContext]()
		{
			device->BeginFrame();
			auto& backBuffer = device->GetCurrentBackBuffer();

			graphicsContext->Reset();
			graphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
			graphicsContext->FlushBarriers();

			graphicsContext->ClearRenderTarget(backBuffer, Color(0.3f, 0.3f, 0.8f));

			graphicsContext->AddBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
			graphicsContext->FlushBarriers();

			device->SubmitContextWork(*graphicsContext);

			device->EndFrame();
			device->Present();
		});

	form.GetEvents().Resize.Connect([&device, &graphicsContext](const Berta::ArgResize& args)
		{
			device->Resize(D3D12Lite::Uint2{ args.NewSize.Width, args.NewSize.Height });

			D3D12_VIEWPORT viewport;
			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;
			viewport.Width = args.NewSize.Width;
			viewport.Height = args.NewSize.Height;
			viewport.MinDepth = D3D12_MIN_DEPTH;
			viewport.MaxDepth = D3D12_MAX_DEPTH;

			graphicsContext->SetViewport(viewport);
		});

	auto formSize = form.GetSize();
	device = std::make_unique<D3D12Lite::Device>(form.Handle()->RootHandle.Handle, D3D12Lite::Uint2{ formSize.Width, formSize.Height });

	graphicsContext = device->CreateGraphicsContext();

	form.Show();
	form.Exec();

	return 0;
}