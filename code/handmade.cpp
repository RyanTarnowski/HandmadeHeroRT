#include "handmade.h"

#define internal static

internal void
RenderWeirdGradient(game_offscreen_Buffer* Buffer, int BlueOffset, int GreenOffset)
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
GameUpdateAndRender(game_offscreen_Buffer *Buffer, int BlueOffset, int GreenOffset)
{
	RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}
