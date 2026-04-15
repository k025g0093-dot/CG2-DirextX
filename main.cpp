#include <Windows.h>
#include <cstdint>

//#pragma warning(push)
//#pragma warning(disable:4023)
//#pragma warning(pop)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	WNDCLASS wc{};
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"MyWindowClass";
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc);

	const int32_t kClineWidth = 1280;
	const int32_t kClineHeight = 720;

	RECT wrc = { 0,0,kClineWidth,kClineHeight };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right-wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	ShowWindow(hwnd, nCmdShow);
	MSG msg{};

	while (msg.message!=WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

		}


	}

	return 0;
}
