//This is an include guard to ensure we can't include this h file twice
#if !defined(HANDMADE_H)

/*
	Services that the platform layer provides to the game.
*/



/*
	Services that the game provides to the platform layer.
*/

struct game_offscreen_Buffer
{
	// Pixels are always 32-bits wide (Memory Order BB GG RR xx, Little Endian 0x xx RR GG BB)
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

internal void GameUpdateAndRender(game_offscreen_Buffer *Buffer, int BlueOffset, int GreenOffset);

#define HANDMADE_H
#endif
