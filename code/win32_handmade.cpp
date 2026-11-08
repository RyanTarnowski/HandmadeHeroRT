#include <Windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_Buffer
{
	// Pixels are always 32-bits wide (Memory Order BB GG RR xx, Little Endian 0x xx RR GG BB)
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

struct win32_window_dimension
{
	int Width;
	int Height;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_Buffer GlobalBackBuffer;


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_


#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;

#define XInputSetState XInputSetState_


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
Win32LoadXInput(void)
{
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (XInputLibrary)
	{
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
	}
	else
	{
		// TODO: Diagnostic log
	}
}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
	// Load library
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
	if (DSoundLibrary)
	{
		// Get DirectSound object
		direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

		LPDIRECTSOUND DirectSound;
		if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
		{
			WAVEFORMATEX WaveFormat = {};
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels = 2;
			WaveFormat.nSamplesPerSec = SamplesPerSecond;
			WaveFormat.wBitsPerSample = 16;
			WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
			WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
			WaveFormat.cbSize = 0;

			if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

				// "Create" a primary buffer
				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
				{
					if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
					{
						OutputDebugStringA("Primary buffer format was set.\n");
					}
					else
					{
						// TODO: Diagnostic log
					}
				}
				else
				{
					// TODO: Diagnostic log
				}
			}
			else
			{
				// TODO: Diagnostic log
			}
				
			DSBUFFERDESC BufferDescription = {};
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.dwFlags = 0;
			BufferDescription.dwBufferBytes = BufferSize;
			BufferDescription.lpwfxFormat = &WaveFormat;

			// "Create" a secondary buffer
			LPDIRECTSOUNDBUFFER SecondaryBuffer;
			if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryBuffer, 0)))
			{
				OutputDebugStringA("Secondary buffer created successfully.\n");
			}
		}
		else
		{
			// TODO: Diagnostic log
		}
	}
	else
	{
		// TODO: Diagnostic log
	}
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return Result;
};

internal void
RenderWeirdGradient(win32_offscreen_Buffer *Buffer, int BlueOffset, int GreenOffset)
{
	uint8* Row = (uint8*)Buffer->Memory;
	for (int Y = 0; Y < Buffer->Height; ++Y)
	{
		uint32* Pixel = (uint32*)Row;
		for (int X = 0; X < Buffer->Width; ++X)
		{
			uint8 Blue = (X + BlueOffset);
			uint8 Green = (Y + GreenOffset);
			uint8 Red = 0;
			uint8 Alpha = 0;

			// Shifting Green, Red, Alpha bits up by 8 then bitwise OR them together
			*Pixel++ = ((Alpha << 24) | (Red << 16) | (Green << 8) | Blue);
		}
		Row += Buffer->Pitch;
	}
}

internal void
Win32ResizeDIBSection(win32_offscreen_Buffer *Buffer, int Width, int Height)
{
	// Free the BitmapMemory before we assign to it
	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	int BytesPerPixel = 4;

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->Pitch = Width * BytesPerPixel;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height; //convert to negative to draw lines from top down
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	int	BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	// TODO: Clear this to black
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_Buffer *Buffer , HDC DeviceContext, int WindowWidth, int WindowHeight)
{
	// TODO: Aspect ratio correction
	StretchDIBits(DeviceContext,  
				 /*
				  X, Y, Width, Height,
				  X, Y, Width, Height,
				 */
				  0, 0, WindowWidth, WindowHeight,
				  0, 0, Buffer->Width, Buffer->Height,
				  Buffer->Memory,
				  &Buffer->Info,
				  DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT CALLBACK 
Win32MainWindowCallback(HWND Window,
					UINT Message,
					WPARAM WParam,
					LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
		case WM_SIZE:
		{
			//OutputDebugStringA("WM_SIZE\n");
		} break;

		case WM_CLOSE:
		{
			GlobalRunning = false;
			//OutputDebugStringA("WM_CLOSE\n");
		} break;

		case WM_ACTIVATEAPP:
		{
			//OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;
		
		case WM_DESTROY:
		{
			GlobalRunning = false;
			//OutputDebugStringA("WM_DESTROY\n");
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 VKCode = WParam;
			bool WasDown = ((LParam & (1 << 30)) != 0);
			bool IsDown = ((LParam & (1 << 31)) == 0);

			if (WasDown != IsDown)
			{
				if (VKCode == 'W')
				{
				}
				else if (VKCode == 'A')
				{
				}
				else if (VKCode == 'S')
				{
				}
				else if (VKCode == 'D')
				{
				}
				else if (VKCode == 'Q')
				{
				}
				else if (VKCode == 'E')
				{
				}
				else if (VKCode == VK_UP)
				{
				}
				else if (VKCode == VK_LEFT)
				{
				}
				else if (VKCode == VK_DOWN)
				{
				}
				else if (VKCode == VK_RIGHT)
				{
				}
				else if (VKCode == VK_ESCAPE)
				{
					OutputDebugStringA("ESCAPE:");
					if (IsDown)
					{
						OutputDebugStringA("IsDown ");
					}
					if (WasDown)
					{
						OutputDebugStringA("WasDown");
					}
					OutputDebugStringA("\n");
				}
				else if (VKCode == VK_SPACE)
				{
				}
			}

			bool32 AltKeyWasDown = (LParam & (1 << 29));
			if ((VKCode == VK_F4) && AltKeyWasDown)
			{
				GlobalRunning = false;
			}

		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			win32_window_dimension Dimension = Win32GetWindowDimension(Window);
			Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
			EndPaint(Window, &Paint);
			//OutputDebugStringA("WM_PAINT\n");
		} break;

		default:
		{
			Result = DefWindowProc(Window, Message, WParam, LParam);
			//OutputDebugStringA("default\n");
		} break;
	}

	return (Result);
}

int CALLBACK 
WinMain(HINSTANCE Instance,
		HINSTANCE PrevInstance,
		LPSTR CommandLine,
		int ShowCode)
{
	Win32LoadXInput();

	WNDCLASSA WindowClass = {};

	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	WindowClass.style = CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClassA(&WindowClass))
	{
		HWND Window =
			CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Handmade Hero",
				WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0);
		if (Window)
		{
			int XOffset = 0;
			int YOffset = 0;


			Win32InitDSound(Window, 48000, 48000*sizeof(int16)*2);

			GlobalRunning = true;
			while(GlobalRunning)
			{
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}

					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}

				// TODO: Should we poll this more frequently
				for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++)
				{
					XINPUT_STATE ControllerState;
					if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
					{
						// This controller is plugged in
						// TODO: See if ControllerState.dwPacketNumber increments too rapidly
						XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;

						bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
						bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
						bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
						bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
						bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 StickX = Pad->sThumbLX;
						int16 StickY = Pad->sThumbLY;

						//if (AButton)
						//{
						//	YOffset += 2;
						//}
						//if (YButton)
						//{
						//	YOffset -= 2;
						//}
						//if (BButton)
						//{
						//	XOffset += 2;
						//}
						//if (XButton)
						//{
						//	XOffset -= 2;
						//}

						XOffset += StickX >> 12;
						YOffset += StickY >> 12;
					}
					else
					{
						// The controller is not available
					}
				}

				//// Vibration test
				//XINPUT_VIBRATION Vibration;
				//Vibration.wLeftMotorSpeed = 60000;
				//Vibration.wRightMotorSpeed = 60000;
				//XInputSetState(0, &Vibration);

				RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

				HDC DeviceContext = GetDC(Window);
				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
				ReleaseDC(Window, DeviceContext);
			}
		}
		else
		{
			// TODO: Logging
		}
	}
	else
	{
		// TODO: Logging
	}

	return(0);
}
