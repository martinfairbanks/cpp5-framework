/*	Perlin noise
	Perlin noise produces a naturally ordered (smooth) sequence of pseudo-random numbers.
	We can think of one-dimensional Perlin noise as a linear sequence of values over time. 
	These examples uses an implementation of Perlin Simplex Noise.
	
	noise() returns a value between 0 and 1.

	Some of these exeamples are inspired by the intro chapter of Daniel Schiffman's excellent book The Nature of Code
	http://natureofcode.com/

	Copyright (c) 2020 Martin Fairbanks
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/

#include "../cpp5_framework.h"

struct Mover
{
	v2 pos;
	v2 time{0, 10000};

	void update()
	{
		pos.x = map(noise(time.x), 0.f, 1.f, 0.f, (f32)width);
		pos.y = map(noise(time.y), 0.f, 1.f, 0.f, (f32)height);

		// move forward through "time"
		time += v2{ 0.01f, 0.01f };
	}

	void draw()
	{
		circle(pos.x, pos.y, 15.f);
	};

} mover;

struct Walker
{
	f32 x, y;
	//if tx = ty the walker would just move diagonaly
	f32 tx = 0;
	f32 ty = 10000;  

	f32 prevX, prevY;

	void update()
	{
		prevX = x;
		prevY = y;

		//move according to noise values
		x = map(noise(tx), 0.f, 1.f, 0.f, (f32)width);
		y = map(noise(ty), 0.f, 1.f, 0.f, (f32)height);

		tx += 0.01f;
		ty += 0.01f;
	}

	void draw()
	{
		stroke(white);
		line(prevX, prevY, x, y);
	}

} walker;

void setup()
{
	createCanvas(960, 540, "Perlin noise");
	background(c64blue);
	fill(green);
	walker.x = (f32)width / 2.f; //center.x;
	walker.y = (f32)height / 2.f; //enter.y;
	disableDoubleBuffer();
}

void draw()
{
	static i32 scene = 0;
	f32 x, y, n, xoff;
	static f32 time = 0.f;
	static f32 increment = 0.02f;

	switch (scene)
	{
	case 0:
		// Perlin noise movement
		background(c64blue);
		mover.update();
		mover.draw();
		setWindowTitle("Perlin noise movement");
		break;

	case 1:
		// Perlin noise walker
		walker.update();
		walker.draw();
		setWindowTitle("Perlin noise walker");
		break;
		
	case 2:
		n = noise(time);
		// map = value,current min, current max, new min, new max
		x = map(n, 0.f, 1.f, 0.f, (f32)width);
		noStroke();
		fill(green, 10);
		circle(x, 250.f, 20.f);
		time += 0.01f;
		setWindowTitle("Perlin noise");
		break;

	case 3:
		// this draws a curve with the x-axis representing time
		background(teal);
		xoff = time;
		noFill();
		
		//beginShape();
		for (int i = 0; i < width; i++)
		{
			y = noise(xoff)* height;
			xoff += 0.001f;
			//vertex(i, y);
			point(i, (i32)y);
		}
		//endShape();
		time += 0.02f;
		setWindowTitle("Perlin noise curve");
		break;

	case 4:
		colorMode(HSB);
		xoff = 0.f; // Start xoff at 0

		//calculate a noise values for every x,y coordinate in a 2D space
		for (int xP = 0; xP < width; xP++)
		{
			xoff += increment;   
			float yoff = 0.0;
			for (int yP = 0; yP < height; yP++)
			{
				yoff += increment;

				//calculate noise and scale by 255
				f32 hue = noise(xoff, yoff) * 255;
				f32 bright = noise(xoff + 1000, yoff + 500, xoff) * 255;

				stroke((i32)hue, 255, (i32)bright);
				//pixelBuffer[x + y*windowWidth] = getColor(Color{ (i32)hue, 255,  (i32)bright });
				point(xP, yP);
			}
		}
		setWindowTitle("2D Perlin noise");
		colorMode(RGB);
		break;
	}

	if (mouseReleased())
	{
		background(c64blue);
		scene++;
		if (scene == 5)
		{
			fill(green);
			scene = 0;
		}
	}
}

void cleanup() {}
