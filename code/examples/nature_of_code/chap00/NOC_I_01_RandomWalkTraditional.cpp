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
		i32 choice = random(3);

		if (choice == 0)
			x++;
		else if (choice == 1)
			x--;
		else if (choice == 2)
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
	createCanvas(960, 540, "Random walker");
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
