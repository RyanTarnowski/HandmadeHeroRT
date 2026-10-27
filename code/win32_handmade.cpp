#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					PWSTR pCmdLine,
					int nCmdShow)
{
	MessageBoxA(0, "This is Handmade Hero", "Handmade Hero", MB_OK | MB_ICONINFORMATION);

	return (0);
}

