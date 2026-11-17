//This is an include guard to ensure we can't include this h file twice
#if !defined(HANDMADE_H)


// Adding typedefs here for code highlighting across all files, consider making a handmade_types.h / handmade_common.h
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static
#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

struct game_offscreen_Buffer
{
	// Pixels are always 32-bits wide (Memory Order BB GG RR xx, Little Endian 0x xx RR GG BB)
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

struct game_sound_output_buffer
{
	int SamplesPerSecond;
	int SampleCount;
	int16* Samples;
};




/*
	Services that the platform layer provides to the game.
*/



/*
	Services that the game provides to the platform layer.
*/

internal void GameUpdateAndRender(game_offscreen_Buffer *Buffer, int BlueOffset, int GreenOffset,
	game_sound_output_buffer *SoundBuffer, int ToneHz);

#define HANDMADE_H
#endif
