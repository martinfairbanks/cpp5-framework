/*	Random walkers and probability
	Some of these exeamples are inspired by Daniel Schiffman's excellent book The Nature of Code
	http://natureofcode.com/

	Copyright (c) 2020 Martin Fairbanks
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/

#include "../cpp5_framework.h"

void setup()
{
	createCanvas(960, 540, "random walkers");
	disableDoubleBuffer();
	background(c64blue);
	fill(magenta);
}

void draw()
{
	stroke(magenta);
	static i32 scene = 0;
	static i32 x = width / 2, y = height / 2;
	static i32 stepX, stepY, prevX, prevY;
	static f32 stepXf, stepYf;
	static i32 x2 = 100, y2 = 200;
	i32 choice;
	f32 randomVal, stepsize;
	
	// a probability of 1%
	static float prob = 0.01f;

	switch (scene)
	{
	case 0:
		// Random walker that moves randomly in four different directions

		// random number between 0-3
		choice = random(3);

		// 1 in 4 (25%) chance the walker will take any given step
		if (choice == 0)
			x++;
		else if (choice == 1)
			x--;
		else if (choice == 2)
			y++;
		else
			y--;
		
		point(x, y);
		setWindowTitle("random walker - four directions");
		break;
		
	case 1:
		// Random walker that moves randomly in eight different directions + standing still
		
		// numbers are between -1 and 1
		// three possible steps along the x- and y-axis (-1, 0, 1) = nine choices
		// 1 in 9 (or 11.1%) chance the walker will take any given step

//		stepX = (rand() % 3) - 1;
//		stepY = (rand() % 3) - 1;
		stepX = random(-1, 1);
		stepY = random(-1, 1);
		x += stepX;
		y += stepY;
		if (stepX < -1 || stepX > 1)
			return;
		point(x, y);
		setWindowTitle("random walker with nine different choices");
		break;

	case 2:
		// Random walker that moves randomly in four different directions, with a 40% chance to move to the left

		// random floating point value between 0 and 1
		randomVal = random();

		if (randomVal < 0.2)		// value between 0 and 0.2 = 20% chance to move to the right
			x++;
		else if (randomVal < 0.6)	// between 0.2 and 0.6 = 40% chance to move to the left
			x--;
		else if (randomVal < 0.8)	// between 0.6 and 0.8 = 20% chance
			y++;
		else						// greater than 0.8 = 20% chance
			y--;

		point(x, y);
		setWindowTitle("random walker with a 40% chance of moving to the left");
		break;

	case 3:
		// 2 random walkers, one has a 40% chance to move to the right
		// the other has 1% chance to walk at all and then 40% chance to move to the right

		// random floating point value between 0 and 1
		randomVal = random();

		if (randomVal < 0.4)
			x++;
		else if (randomVal < 0.6)
			x--;
		else if (randomVal < 0.8)
			y++;
		else
			y--;

		point(x, y);

		// if the random value is less than 0.01
		if (randomVal < prob)
		{
			randomVal = random();
			if (randomVal < 0.4)
				x2++;
			else if (randomVal < 0.6)
				x2--;
			else if (randomVal < 0.8)
				y2++;
			else
				y2--;

			stroke(green);
			point(x2, y2);
		}

		setWindowTitle("2 random walkers");
		break;

	case 4:
		// Random walker: Levy flight

		choice = random(3);

		if (choice == 0)
			x++;
		else if (choice == 1)
			x--;
		else if (choice == 2)
			y++;
		else if (choice == 3)
			y--;
		
		point(x, y);

		// random floating point value between 0 and 1
		randomVal = random();

		if (randomVal > 0.9)
		{
			stepX = random(-10, 10); 
			stepY = random(-10, 10);

			line(x, y, x + stepX, y + stepY);
			x = x + stepX;
			y = y + stepY;
		}
		setWindowTitle("random walker: Levy flight");
		break;

	case 5:
		// Random walker: Levy flight with Monte Carlo algorithm
		prevX = x;
		prevY = y;

		stepXf = random(-1.f, 1.f);
		stepYf = random(-1.f, 1.f);

		stepsize = montecarlo()*50;
		stepXf *= stepsize;
		stepYf *= stepsize;

		x += (i32)stepXf;
		y += (i32)stepYf;

		x = constrain(x, 0, width - 1);
		y = constrain(y, 0, height - 1);

		line(prevX, prevY, x, y);

		setWindowTitle("random walker: Levy flight with Monte Carlo algorithm");
		break;
	}
	
	if (mouseReleased())
	{
		background(c64blue);
		scene++;
		x = width / 2;
		y = height / 2;
		if (scene == 6)
			scene = 0;
	}

}

void cleanup() {}
