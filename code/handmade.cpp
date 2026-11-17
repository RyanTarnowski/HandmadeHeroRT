#include "handmade.h"

// Added for sinf
#include "math.h"

internal void
GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz)
{
	local_persist real32 tSine;
	int16 ToneVolume = 3000;
	int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

	int16 *SampleOut = SoundBuffer->Samples;

	for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
	{
		// Square wave
		//int16 SampleValue = ((RunningSampleIndex++ / HalfWavePeriod) % 2) ? ToneVolume : -ToneVolume;

		// Sine wave
		//real32 t = 2.0f * Pi32 * 1.0f * (real32)SoundOutput->RunningSampleIndex / (real32)SoundOutput->WavePeriod;
		real32 SineValue = sinf(tSine);
		int16 SampleValue = (int16)(SineValue * ToneVolume);
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;

		tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
	}
}

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
GameUpdateAndRender(game_offscreen_Buffer *Buffer, int BlueOffset, int GreenOffset,
	game_sound_output_buffer *SoundBuffer, int ToneHz)
{
	// TODO: Allow sample offsets here for more robust platform options
	GameOutputSound(SoundBuffer, ToneHz);

	RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}
