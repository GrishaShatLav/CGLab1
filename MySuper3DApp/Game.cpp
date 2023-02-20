#include "Game.h"

Game* Game::instance = nullptr;

/*
* Static method for handling user input
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
	return Game::instance->MessageHandler(hwnd, umessage, wparam, lparam);
}

LRESULT Game::MessageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
	switch (umessage) {
	case WM_INPUT: {
		UINT dwSize = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];

		if (lpb == nullptr) 
			return 0;

		if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb);

		if (raw->header.dwType == RIM_TYPEKEYBOARD) {
			//printf(" Kbd: make=%04i Flags:%04i Reserved:%04i ExtraInformation:%08i, msg=%04i VK=%i \n",
			//	raw->data.keyboard.MakeCode,
			//	raw->data.keyboard.Flags,
			//	raw->data.keyboard.Reserved,
			//	raw->data.keyboard.ExtraInformation,
			//	raw->data.keyboard.Message,
			//	raw->data.keyboard.VKey);

			inputDevice.get()->OnKeyDown({
				raw->data.keyboard.MakeCode,
				raw->data.keyboard.Flags,
				raw->data.keyboard.VKey,
				raw->data.keyboard.Message
				});
		}
		else if (raw->header.dwType == RIM_TYPEMOUSE) {
			//printf(" Mouse: X=%04d Y:%04d \n", raw->data.mouse.lLastX, raw->data.mouse.lLastY);
			inputDevice->OnMouseMove({
				raw->data.mouse.usFlags,
				raw->data.mouse.usButtonFlags,
				static_cast<int>(raw->data.mouse.ulExtraInformation),
				static_cast<int>(raw->data.mouse.ulRawButtons),
				static_cast<short>(raw->data.mouse.usButtonData),
				raw->data.mouse.lLastX,
				raw->data.mouse.lLastY
				});
		}

		delete[] lpb;
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
	default: {
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
	}
}

Game::Game(LPCWSTR name, int clientWidth, int clientHeight, bool windowed) {
	this->name = name;
	this->clientWidth = clientWidth;
	this->clientHeight = clientHeight;
	this->windowed = windowed;

	totalTime = 0;
	deltaTime = 0;
	frameCount = 0;
	startTime = std::make_shared<std::chrono::time_point<std::chrono::steady_clock>>();
	prevTime = std::make_shared<std::chrono::time_point<std::chrono::steady_clock>>();

	viewport = std::make_shared<D3D11_VIEWPORT>();
	swapDesc = std::make_shared<DXGI_SWAP_CHAIN_DESC>();
}

/*
* There is no public access to constructor because of "Singleton" pattern
* Need to use this method to create Game::instance
*/
void Game::CreateInstance(LPCWSTR name, int screenWidth, int screenHeight, bool windowed) {
	if (!instance)
		instance = new Game(name, screenWidth, screenHeight, windowed);
}

/*
* Prepare all "Game" resources
*/
void Game::PrepareResources() {
	display = std::make_shared<DisplayWin32>(name, clientWidth, clientHeight, WndProc);
	inputDevice = std::make_shared<InputDevice>();

	// Initialize viewport parameters
	viewport->TopLeftX = 0; // X position of the left hand side of the viewport
	viewport->TopLeftY = 0; // Y position of the top of the viewport
	viewport->Width = static_cast<float>(display->GetClientWidth()); // Width of the viewport
	viewport->Height = static_cast<float>(display->GetClientHeight()); // Height of the viewport
	viewport->MinDepth = 0; // Minimum depth of the viewport. Ranges between 0 and 1
	viewport->MaxDepth = 1.0f; // Maximum depth of the viewport. Ranges between 0 and 1

	// BufferDesc describes the backbuffer display mode
	swapDesc->BufferDesc.Width = display->GetClientWidth(); // Resolution width
	swapDesc->BufferDesc.Height = display->GetClientHeight(); // Resolution height
	swapDesc->BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Display format (32-bit unsigned normalized integer format supporting 8 bits per channel, including the alpha channel)
	swapDesc->BufferDesc.RefreshRate.Numerator = 60; // Refresh rate in hertz numerator
	swapDesc->BufferDesc.RefreshRate.Denominator = 1; // Refresh rate in hertz denominator (for representing integer it = 1)
	swapDesc->BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // Scanline drawing mode (indicating the method the raster uses to create an image on a surface)
	swapDesc->BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // Scaling mode (indicating how an image is stretched to fit a given monitor's resolution)

	/* 
	* SampleDesc describes multi - sampling parameters for a resource
	* The default sampler mode, with no anti-aliasing, has a count of 1 and a quality level of 0
	*/
	swapDesc->SampleDesc.Count = 1; // Number of multisamples per pixel
	swapDesc->SampleDesc.Quality = 0; // The image quality level. The higher the quality, the lower the performance

	swapDesc->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Describes the surface usage and CPU access options for the back buffer
	swapDesc->BufferCount = 2; // Number of buffers in the swap chain (double or triple buffering)
	swapDesc->OutputWindow = display->GetHWnd(); // Handle to the output window. This member must not be NULL.
	swapDesc->Windowed = windowed;
	swapDesc->SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Describes options for handling the contents of the presentation buffer after presenting a surface
	swapDesc->Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Options for swap-chain behavior

	const int featureLevelsNumber = 1;
	D3D_FEATURE_LEVEL featureLevels[featureLevelsNumber] = { D3D_FEATURE_LEVEL_11_1 };

	// Creates a device that represents the display adapter and a swap chain used for rendering
	res = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		featureLevels, // Determine the order of feature levels to attempt to create
		featureLevelsNumber, // The number of elements in feature levels array
		D3D11_SDK_VERSION,
		swapDesc.get(),
		swapChain.GetAddressOf(),
		device.GetAddressOf(),
		nullptr, // Feature level for device
		context.GetAddressOf()
	);

	if (FAILED(res)) {
		// Well, that was unexpected
	}
	
	res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backTex.GetAddressOf()); // Accesses one of the buffers of the back buffer chain
	res = device->CreateRenderTargetView(backTex.Get(), nullptr, rtv.GetAddressOf());
}

/*
* Initialize all "GameComponent" items in vector
*/
void Game::Initialize() {
	for (auto component : gameObjects)
		component->Initialize();
}

/*
* Prepare next frame
* Clear states and render targets
*/
void Game::PrepareFrame() {
	context->ClearState(); // Reset parameters to default

	context->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);

	context->RSSetViewports(1, viewport.get());
	float backgroundColor[] = { 0.18f, 0.55f, 0.34f, 1.0f };
	context->ClearRenderTargetView(rtv.Get(), backgroundColor);
}

/*
* Update all "GameComponent" items in vector
* For logic
*/
void Game::Update() {
	for (auto gameObject : gameObjects)
		gameObject->Update();

	// Handle ESC button
	if (inputDevice->IsKeyDown(Keys::Escape))
		PostQuitMessage(0);
}

/*
* FixedUpdate all "GameComponent" items in vector
* For physics
*/
void Game::FixedUpdate() {
	for (auto gameObject : gameObjects)
		gameObject->FixedUpdate();
}

/*
* Draw all "GameComponent" items in vector
*/
void Game::Draw() {
	for (auto gameObject : gameObjects)
		gameObject->Draw();
}

/*
* There is still no understanding of what will happen here
*/
void Game::RestoreTargets() {

}

/*
* Presenting graphics
*/
void Game::EndFrame() {
	context->OMSetRenderTargets(0, nullptr, nullptr);

	swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0); // Show what we've drawn
}

/*
* Internal method
* Update all items
* Call each frame
*/
void Game::UpdateInternal() {
	auto curTime = std::chrono::steady_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - *prevTime).count() / 1000000.0f;
	*prevTime = curTime;

	totalTime += deltaTime;
	frameCount++;

	if (totalTime > 1.0f) {
		float fps = frameCount / totalTime;

		totalTime -= 1.0f;

		WCHAR text[256];
		swprintf_s(text, TEXT("FPS: %f"), fps);
		SetWindowText(display->GetHWnd(), text);

		frameCount = 0;
	}

	PrepareFrame();

	Update();

	Draw();

	RestoreTargets();

	EndFrame();
}

/*
* Main method for starting game with initialization
*/
void Game::Run() {
	PrepareResources();
	Initialize();

	*startTime = std::chrono::steady_clock::now();
	*prevTime = *startTime;
	
	MSG msg = {};
	while (msg.message != WM_QUIT) {
		// Handle the windows messages.
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			UpdateInternal();
	}
	
	DestroyResources();
}

/*
* Exit game with destroying resources
*/
void Game::Exit() {
	DestroyResources();
}

void Game::DestroyResources() {
	for (auto gameObject : gameObjects)
		gameObject->DestroyResources();
}


std::shared_ptr<DisplayWin32> Game::GetDisplay() {
	return display;
}

Microsoft::WRL::ComPtr<ID3D11Device> Game::GetDevice() {
	return device;
}

Microsoft::WRL::ComPtr<ID3D11RenderTargetView> Game::GetRTV() {
	return rtv;
}

Microsoft::WRL::ComPtr <IDXGISwapChain> Game::GetSwapChain() {
	return swapChain;
}

Microsoft::WRL::ComPtr <ID3D11DeviceContext> Game::GetContext() {
	return context;
}
