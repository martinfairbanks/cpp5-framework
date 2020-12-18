/* 	Falling snowfakes

	Copyright (c) 2020 Martin Fairbanks
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/

#include "../cpp5_framework.h"

#define MAX_FLAKES	2000		
#define MAX_LAYERS	5

struct SnowFlake
{
	i32 x, y;
	i32 layer;
} snow[2000];

void setup()
{
	createCanvas(960, 540, "Snowflakes");
	stroke(white);

	//init snowflakes
	for (int i = 0; i < arrayCount(snow); i++)
	{
		snow[i].x = rand() % width;
		snow[i].y = rand() % height;
		snow[i].layer = rand() % MAX_LAYERS;
	}
}

void draw()
{
	clear(c64blue);

	for (int i = 0; i < MAX_FLAKES; i++)
	{
		//move snowflake down depending on layer
		snow[i].y += snow[i].layer + 1;

		//check if snowflake moved outside of screen
		if (snow[i].y > height)
		{
			snow[i].x = rand() % width;
			snow[i].y = 0;
			snow[i].layer = rand() % MAX_LAYERS;
		}

		//shake and draw
		snow[i].x = (snow[i].x + (rand() % 5) - 2);
		point((int)snow[i].x, (int)snow[i].y);
	}
}

void cleanup() { }