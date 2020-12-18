/*	Random walker
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
	i32 x, y;

	Walker()
	{
		x = width / 2;
		y = height / 2;
	}

	void render()
	{
		point(x, y);
	}

	void step()
	{
		//random floating point value between 0 and 1
		f32 choice = random();

		// 40% chance of moving to the right
		if (choice < 0.4)
			x++;
		else if (choice < 0.6)
			x--;
		else if (choice < 0.8)
			y++;
		else
			y--;
	
		x = constrain(x, 0, width - 1);
		y = constrain(y, 0, height - 1);
	}
};

Walker *walker;

void setup()
{
	createCanvas(960, 540, "Random walker 40% chance of moving to the right");
	disableDoubleBuffer();
	walker = new Walker();
	background(c64blue);
	stroke(magenta);
}

void draw()
{
	walker->step();
	walker->render();
}

void cleanup()
{
	delete walker;
}
