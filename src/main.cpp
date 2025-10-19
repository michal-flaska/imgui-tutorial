// These are the main ImGui headers we need
#include "imgui.h"
#include "imgui_impl_win32.h"  // ImGui's Windows implementation
#include "imgui_impl_dx11.h"   // ImGui's DirectX 11 implementation

// DirectX 11 stuff - this is what actually draws things on screen
#include <d3d11.h>
#include <tchar.h>

// These are our DirectX objects - they handle all the rendering
// Think of them as the tools we need to draw on the screen
static ID3D11Device* g_pd3dDevice = nullptr;           // Main D3D11 device
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;    // Used to issue drawing commands
static IDXGISwapChain* g_pSwapChain = nullptr;           // Manages front/back buffers (double buffering)
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr; // Where we actually render to

// Helper functions (we'll define these below)
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main entry point
int main(int, char**)
{
	// === STEP 1: CREATE THE WINDOW ===
	// This is just a regular Windows window, nothing ImGui-specific yet

	WNDCLASSEXW wc = {
	    sizeof(wc),                     // Size of this structure
	    CS_CLASSDC,                     // Class style
	    WndProc,                        // Our message handler function
	    0L, 0L,                         // Extra class/window memory
	    GetModuleHandle(nullptr),       // Handle to this program
	    nullptr, nullptr, nullptr, nullptr,
	    L"ImGui Example",               // Window class name
	    nullptr
	};

	::RegisterClassExW(&wc);

	// Actually create the window
	HWND hwnd = ::CreateWindowW(
		wc.lpszClassName,               // Class name we just registered
		L"Dear ImGui DirectX11",        // Window title (you can change this!)
		WS_OVERLAPPEDWINDOW,            // Window style (normal window with borders, minimize, etc)
		100, 100,                       // Position on screen (x, y)
		1280, 800,                      // Size (width, height)
		nullptr, nullptr,
		wc.hInstance,
		nullptr
	);

	// === STEP 2: INITIALIZE DIRECTX 11 ===
	// DirectX needs to be set up before ImGui can use it

	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// === STEP 3: INITIALIZE IMGUI ===

	IMGUI_CHECKVERSION();           // Make sure we're using the right version
	ImGui::CreateContext();         // Create ImGui context (required!)
	ImGuiIO& io = ImGui::GetIO();   // Get IO object (handles input/output)
	(void)io;                       // We're not using it yet, but we will later

	// You can configure ImGui here
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable gamepad controls

	// Set the ImGui style/theme - try ImGui::StyleColorsLight() or ImGui::StyleColorsClassic()
	ImGui::StyleColorsDark();

	// Tell ImGui to use our Windows window and DirectX device
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	// === STEP 4: OUR PROGRAM STATE ===
	// These are just example variables to show how ImGui controls work

	bool show_demo_window = true;       // Should we show the demo window?
	bool show_another_window = false;   // Should we show our custom window?
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);  // Background color (RGBA)

	// === STEP 5: MAIN LOOP ===
	// This runs every frame until you close the window

	bool done = false;
	while (!done)
	{
		// Handle Windows messages (like clicking the X button, resizing, etc)
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// === START NEW IMGUI FRAME ===
		// This needs to be done every frame before we create any UI

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// === CREATE YOUR UI HERE ===

		// Show the big demo window
		// This has TONS of examples - study it to learn what ImGui can do!
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// Create a simple custom window
		{
			static float f = 0.0f;              // Static means this value persists between frames
			static int counter = 0;

			ImGui::Begin("Hello, world!");      // Start a new window with this title

			ImGui::Text("This is some text.");  // Display text

			// Checkbox - automatically updates the bool when clicked
			ImGui::Checkbox("Demo Window", &show_demo_window);
			ImGui::Checkbox("Another Window", &show_another_window);

			// Slider - lets you drag to change the value
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

			// Color picker - changes the background color
			ImGui::ColorEdit3("clear color", (float*)&clear_color);

			// Button - returns true when clicked
			if (ImGui::Button("Button"))
				counter++;

			ImGui::SameLine();  // Put the next widget on the same line
			ImGui::Text("counter = %d", counter);

			// Display framerate
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
				1000.0f / io.Framerate, io.Framerate);

			ImGui::End();  // End the window
		}

		// Another custom window (only shows if checkbox is checked)
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// === RENDER EVERYTHING ===

		ImGui::Render();  // Finalize ImGui's draw data

		// Clear the screen with our background color
		const float clear_color_with_alpha[4] = {
		    clear_color.x * clear_color.w,
		    clear_color.y * clear_color.w,
		    clear_color.z * clear_color.w,
		    clear_color.w
		};
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);

		// Actually draw ImGui's stuff
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Present the frame (swap front/back buffers)
		g_pSwapChain->Present(1, 0); // 1 = vsync on, 0 = vsync off
	}

	// === CLEANUP ===
	// When we exit, we need to free all resources

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}

// === HELPER FUNCTIONS ===

// Creates the DirectX 11 device and swap chain
bool CreateDeviceD3D(HWND hWnd)
{
	// Set up the swap chain description
	// Swap chain = manages the front and back buffers for rendering
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;                                 // Double buffering
	sd.BufferDesc.Width = 0;                            // Auto-detect from window size
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Color format
	sd.BufferDesc.RefreshRate.Numerator = 60;           // 60 Hz
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;                            // No anti-aliasing
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;                                 // Not fullscreen
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	// createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;  // Uncomment for debug mode

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

	// Actually create the device
	HRESULT res = D3D11CreateDeviceAndSwapChain(
		nullptr,                    // Use default adapter
		D3D_DRIVER_TYPE_HARDWARE,   // Use GPU
		nullptr,                    // No software rasterizer
		createDeviceFlags,
		featureLevelArray,
		2,
		D3D11_SDK_VERSION,
		&sd,
		&g_pSwapChain,
		&g_pd3dDevice,
		&featureLevel,
		&g_pd3dDeviceContext
	);

	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	// Get the back buffer from the swap chain and create a render target view
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Windows message handler
// This gets called whenever Windows sends a message to our window
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Let ImGui handle the message first (for mouse/keyboard input)
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:  // Window was resized
		if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
		{
			// Recreate render target with new size
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT menu
			return 0;
		break;
	case WM_DESTROY:  // Window is closing
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
