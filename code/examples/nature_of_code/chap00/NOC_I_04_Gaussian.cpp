/*	Random Gaussian
	This exeample is from the introduction chapter of Daniel Schiffman's book The Nature of Code.
	
	The Nature of Code
	Daniel Shiffman
	http://natureofcode.com
	
	C++ port by Martin Fairbanks.
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/

#include "../../../cpp5_framework.h"

void setup()
{
	createCanvas(960, 540, "Gaussian number distribution");
	disableDoubleBuffer();
	background(220);
}

void draw()
{
	// Get gaussian random numbers with a mean of 0 and standard deviation of 1.0
	f32 xloc = randomGaussian();

	// Define a standard deviation of 60
	f32 sd = 60;

	// A mean (average) that is in the middle of the window along the x-axis
	f32 mean = center.x;

	// Scale the gaussian number by standard deviation and mean
	xloc = xloc * sd + mean;

	fill(0, 10);
	noStroke();
	ellipse(xloc, center.y, 16.f);
}

void cleanup() {}
