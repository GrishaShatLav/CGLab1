#pragma once
#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "SimpleMath.h"
#include "SimpleMath.inl"
#include <iostream>
#include <chrono>
#include <vector>

#include "GameObject.h"
#include "DisplayWin32.h"
#include "InputDevice.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

class DisplayWin32;
class RenderComponent;
class InputDevice;

class Game {
protected:
	LPCWSTR	name; // Name of the game application
	int clientWidth;
	int clientHeight;
	bool windowed;

	std::shared_ptr<DisplayWin32> display; // WinApi display
	std::shared_ptr<D3D11_VIEWPORT> viewport; // Defines the dimensions of a viewport
	Microsoft::WRL::ComPtr<ID3D11Device> device; // The device interface represents a virtual adapter and it is used to create resources
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context; // Interface represents a device context which generates rendering commands
	std::shared_ptr<DXGI_SWAP_CHAIN_DESC> swapDesc; // Descriptor, that describes swap chain
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain; // Interface implements one or more "IDXGISurface" for storing rendered data before presenting it to an output
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backTex; // 2D texture interface manages texel data, which is structured memory
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv; // Identifies the render-target subresources that can be accessed during rendering (Back buffer?)

	Game(LPCWSTR name, int clientWidth, int clientHeight, bool windowed);

	void UpdateInternal();
	void DestroyResources();
	void PrepareResources();
	void Initialize();
	void PrepareFrame();
	virtual void Update();
	void FixedUpdate();
	void Draw();
	void EndFrame();

public:
	static Game* instance; // Singleton
	std::vector<GameObject*> gameObjects; // Change to smart pointers
	std::shared_ptr<InputDevice> inputDevice; // For input handling
	std::shared_ptr<std::chrono::time_point<std::chrono::steady_clock>> startTime;
	std::shared_ptr<std::chrono::time_point<std::chrono::steady_clock>> prevTime;
	HRESULT res; // Used for return codes from "Direct3D 11" functions
	float totalTime;
	float deltaTime;
	unsigned int frameCount;

	static void CreateInstance(LPCWSTR name, int screenWidth, int screenHeight, bool windowed);

	void RestoreTargets();
	virtual void Run();
	void Exit();

	LRESULT MessageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);


	std::shared_ptr<DisplayWin32> GetDisplay();

	Microsoft::WRL::ComPtr<ID3D11Device> GetDevice();

	Microsoft::WRL::ComPtr <ID3D11DeviceContext> GetContext();

	Microsoft::WRL::ComPtr <IDXGISwapChain> GetSwapChain();

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRTV();
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
