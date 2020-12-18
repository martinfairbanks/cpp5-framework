/*	Noise walker
	This exeample is from the introduction chapter of Daniel Schiffman's book The Nature of Code.
	
	The Nature of Code
	Daniel Shiffman
	http://natureofcode.com
	
	C++ port by Martin Fairbanks.
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/

#include "../../../cpp5_framework.h"

struct Walker
{
	v2 position;
	v2 noff;

	Walker()
	{
		position = v2((f32)width / 2.f, (f32)height / 2.f);
		noff = v2(random(1000.f), random(1000.f));
	}

	void render()
	{
		fill(0, 32);
		noStroke();
		ellipse(position.x, position.y, 24.f);
	}

	void walk()
	{
		position.x = map(noise(noff.x), 0.f, 1.f, 0.f, (f32)width);
		position.y = map(noise(noff.y), 0.f, 1.f, 0.f, (f32)height);

		// move forward through "time"
		noff += v2{ 0.01f, 0.01f };
	}
};

Walker *walker;

void setup()
{
	createCanvas(960, 540, "Perlin noise walker");
	disableDoubleBuffer();
	walker = new Walker();
	background(220);
}

void draw()
{
	walker->walk();
	walker->render();
}

void cleanup()
{
	delete walker;
}
