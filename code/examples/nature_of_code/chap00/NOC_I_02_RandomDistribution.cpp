/*	Random number distribution
	This exeample is from the introduction chapter of Daniel Schiffman's book The Nature of Code.
	
	The Nature of Code
	Daniel Shiffman
	http://natureofcode.com
	
	C++ port by Martin Fairbanks.
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/

#include "../../../cpp5_framework.h"

// An array to keep track of how often random numbers are picked
i32 randomCounts[53];

void setup()
{
	createCanvas(640, 360, "Pseudo-random uniform distribution of numbers");
	for (i32 i = 0; i < arrayCount(randomCounts); i++)
		randomCounts[i] = 0;
}

void draw()
{
	background(127);
	i32 index = random((i32)arrayCount(randomCounts));
	randomCounts[index]++;

	// Draw a rectangle to graph results
	stroke(0);
	strokeWeight(2);
	fill(255);

	i32 w = width / arrayCount(randomCounts);

	for (i32 x = 0; x < arrayCount(randomCounts); x++)
		rect(x * w, height - randomCounts[x], w - 1, randomCounts[x]);
}

void cleanup() {}