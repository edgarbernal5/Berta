/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#include <Berta/Controls/Form.h>
#include <D3D12Lite.h>

int main()
{
	std::unique_ptr<D3D12Lite::Device> mDevice;
	std::unique_ptr<D3D12Lite::GraphicsContext> mGraphicsContext;

	Berta::Form form(Berta::Size(450u, 350u), { true, true, true }, true);
	form.SetCaption("D3D - Example");

	auto formSize = form.GetSize();
	mDevice = std::make_unique<D3D12Lite::Device>(form.Handle()->RootHandle.Handle, D3D12Lite::Uint2{ formSize.Width, formSize.Height });

	mGraphicsContext = mDevice->CreateGraphicsContext();

	form.Show();
	form.Exec();

	return 0;
}