#include <windows.h>
#include <iostream>

#include "Example.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void* _CreateWindow(int width, int height, HINSTANCE instance)
{
	// Register class for this window
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"SilexUISandbox";

	if (!RegisterClassEx(&wc))
		return nullptr;

	// Create window
	auto wnd = CreateWindowEx(0, wc.lpszClassName, L"SilexUI Sandbox",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		width, height, 0, 0, instance, 0);

	if (!wnd)
		return nullptr;

	// Show window
	ShowWindow(wnd, SW_SHOW);

	return wnd;
}

void AllocateNewConsoleWin32()
{
	// 1. Allocate new console
	AllocConsole();

	// 2. Redirect all text in our new console
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	// 3. Change console title
	SetConsoleTitle(L".Hlsl shaders hot reload");
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	// For debug prints
	AllocateNewConsoleWin32();

	// Create window
	int windowWidth = 1280;
	int windowHeight = 720;
	auto wndPtr = _CreateWindow(windowWidth, windowHeight, hInstance);
	if (!wndPtr)
	{
		printf("Failed create window\n");
		return 1;
	}

	Example app;
	if (!app.Initialize(wndPtr, (float)windowWidth, (float)windowHeight))
	{
		printf("Failed initialize Example class!\n");
		return 1;
	}

	bool isRunning = true;
	MSG msg = {};
	while (isRunning)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				isRunning = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Render example scene
		app.StartFrame();

		app.RenderScene();

		app.EndFrame();
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}