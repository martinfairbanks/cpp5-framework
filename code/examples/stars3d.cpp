/* 	3D starfield

	Copyright (c) 2020 Martin Fairbanks
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/
#include "../cpp5_framework.h"

struct Star
{
	f32 xpos, ypos;
	i32 zpos, speed;
	u8 color;
} stars[2000];

global i32 centerX;
global i32 centerY;
global f32 prevPosX;
global f32 prevPosY;

void initStar(Star* star, int i)
{
	// randomly init stars, put them around the center of the screen
	star->xpos = float(((rand() % width) * -1) + centerX);
	star->ypos = float(((rand() % height) * -1) + centerY);

	// change viewpoint
	if (mouseX > width / 2)
	{
		star->xpos *= 800.0;
		star->ypos *= 800.0;
	}
	else
	{
		star->xpos *= 40.0;
		star->ypos *= 40.0;
	}
		
	star->zpos = i;
	star->speed = (rand() % 10) + 4;
	star->color = u8(i << 1);
}

void setup()
{
	createCanvas(960, 540, "stars3d");
	centerX = width / 2;
	centerY = height / 2;
	
	for (int i = 0; i < arrayCount(stars); i++)
		initStar(stars + i, i + 1);
}

void draw()
{
	clear(black);

	// move and draw stars
	for (int i = 0; i < arrayCount(stars); i++)
	{
		prevPosX = (stars[i].xpos / stars[i].zpos) + centerX;
		prevPosY = (stars[i].ypos / stars[i].zpos) + centerY;

		// change speed depending on mouse pos
		stars[i].speed = mouseY / 20;
		
		// move star closer
		stars[i].zpos -= stars[i].speed;
		
		// compute 3D position
		f32 tempX = (stars[i].xpos / stars[i].zpos) + centerX;
		f32 tempY = (stars[i].ypos / stars[i].zpos) + centerY;

		// check if star has moved outside of screen
		if (tempX < 0 || tempX > width - 1 || tempX < 0 || tempY > height - 1 || stars[i].zpos <= 0)
		{
			initStar(stars + i, i + 1);
			continue;
		}

		if (mouseX > width / 2)
		{
			stroke(white);
			if (tempX == prevPosX || tempY ==prevPosY)
				point((int)tempX, (int)tempY);
			else
				line((int)tempX, (int)tempY, (int)prevPosX, (int)prevPosY);
		}
		else
		{
			stars[i].color = u8(stars[i].zpos >> 1);
			noStroke();
			fill(0, 0, 255, stars[i].color);
			circle((int)tempX, (int)tempY, 4);
		}
	}
}

void cleanup() { }