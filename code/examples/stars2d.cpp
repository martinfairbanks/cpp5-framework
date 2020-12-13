/* 	2D starfield
*
	Copyright (c) 2020 Martin Fairbanks
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/
#include "../cpp5_framework.h"

struct Star
{											
	f32 x, y;							
	u8 colorPlane;							
} stars[600];

enum Direction
{
	STARS_LEFT,
	STARS_RIGHT
};

Direction direction = STARS_RIGHT;
unsigned int starColors[3] = { 0x30, 0x80, 0xff };
f32 speed = 0.6f;

void setup()
{
	createCanvas(960, 540, "stars2d");

	// randomly generate some stars
	for (int i = 0; i < arrayCount(stars); i++)
	{
		stars[i].x = float(rand() % width);
		stars[i].y = float(rand() % height);
		stars[i].colorPlane = rand() % 3; // star color between 0 and 2
	}
}

void draw()
{
	if (mouseX > width / 2)
		direction = STARS_RIGHT;
	else
		direction = STARS_LEFT;

	clear(black);
	
	speed = (float)mouseY / 80.0f;

	if (direction == STARS_RIGHT)
	{
		// move and draw stars
		for (int i = 0; i < arrayCount(stars); i++)
		{
			// move star with a speed depending on plane
			stars[i].x += (1.0f + (float)stars[i].colorPlane)*speed;
		
			// check if star has moved outside of screen
			if (stars[i].x > width)
			{
				stars[i].x = 0;

				// new random y pos
				stars[i].x = float((rand() % 100) * -1.0f);	//to prevent stars from lining up after fast speed
				stars[i].y = float(rand() % height);
			}
			
			stroke(0, starColors[stars[i].colorPlane], 0);
			point((int)stars[i].x, (int)stars[i].y);
		}
	}
	else
	{
		for (int i = 0; i < arrayCount(stars); i++)
		{
			stars[i].x -= (1 + (float)stars[i].colorPlane)*speed;

			if (stars[i].x <= 0)
			{
				stars[i].x = float((rand() % 100) + width);
				stars[i].y = float(rand() % height);
			}
			noStroke();
			fill(0, 0, starColors[stars[i].colorPlane]);
			circle((int)stars[i].x, (int)stars[i].y, 2);
		}
	}
}

void cleanup() { }