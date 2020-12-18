/*	Random numbers by Martin Fairbanks
	Some of these exeamples are inspired by Daniel Schiffman's excellent book The Nature of Code
	http://natureofcode.com/

	Copyright (c) 2020 Martin Fairbanks
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/

#include "../cpp5_framework.h"

void setup()
{
	createCanvas(960, 540, "random uniform and normal distribution of numbers");
	disableDoubleBuffer();
	clear(c64blue);
	fill(blue);
}

void draw()
{
	static i32 scene = 0;
	i32 randomValue1, randomValue2;
	f32 randomFloat;
	f32 sd, mean;

	switch (scene)
	{
	case 0:
		// pseudo-random uniform distribution of numbers
		// random integers between 0 and 3
		// i32 x = random(0, 3);

		// random integers between -1 and 1
		// i32 x = random(-1, 1);

		clear(c64blue);
		// random integers between 0 and 3
		randomValue1 = random(3);
		text(width / 2, height / 2, "%d", randomValue1);
		setWindowTitle("pseudo-random uniform distribution of numbers");
		break;

	case 1:
		//random floating point numbers between 0 and 1
		clear(c64blue);
		randomFloat = random();
		text(width / 2, height / 2, "%f", randomFloat);

		//random floating point numbers between -2 and 2
		//randomFloat = randomf(-2, 2);
		
		setWindowTitle("random floating point numbers between 0 and 1");
		break;

	case 2:
		// gaussian (normal) distribution of random numbers around the mean with a specific standard deviation.
		// the probability of getting values far from the mean is low, the probability of getting numbers near the mean is high.
		
		// gaussian random numbers with a mean of 0 and standard deviation of 1.0
		// randomFloat = randomGaussian();

		clear(c64blue);
		//gaussian random numbers with a mean of 10 and standard deviation of 1.0
		randomFloat = randomGaussian(10.0f, 1.0f);
		text(width / 2, height / 2, "%f", randomFloat);
		setWindowTitle("Gaussian number distribution");
		break;
	
	case 3:
		// random numbers between min and max
		randomValue1 = random(0, width);
		randomValue2 = random(0, height);
		fill(random(255), random(255), random(255), 127);
		ellipse(randomValue1, randomValue2, 8);
		setWindowTitle("random numbers between min and max");
		break;

	case 4:
		// gaussian number distribution
		stroke(green);
		for (i32 y = 0; y < height; y++)
		{
			randomValue1 = (i32)randomGaussian(center.x, 15);
			line((i32)center.x, y, randomValue1, y);
		}
		setWindowTitle("Gaussian number distribution");
		break;

	case 5:
		// gaussian random numbers with a mean of 0 and standard deviation of 1.0
		randomFloat = randomGaussian();

		// standard deviation of 50
		// with a low standard deviation the majority of the values cluster closely around the mean
		sd = 100;

		// a mean (average) that is in the middle of the window along the x-axis
		mean = center.x;

		// scale the gaussian number by standard deviation and mean
		randomFloat = randomFloat * sd + mean;
		
		noStroke();
		fill(red, 10);
		circle((i32)randomFloat, (i32)center.y, 10);
		
		setWindowTitle("Gaussian number distribution");
		break;

	case 6:	
		// pseudo-random uniform distribution of numbers
		// the number with the highest count is shown in red
		static int randomCounts[51];
		static int w;
		i32 highestIndex = 0, highestNumber = 0;

		// number between 0-50
		i32 index = random(50);
		randomCounts[index]++;

		// get the highest number
		for (int x = 0; x <= 50; x++)
		{
			if (randomCounts[x] > highestNumber)
			{
				highestNumber = randomCounts[x];
				highestIndex = x;
			}
		}

		// draw 50 bars
		w = width / 50;

		for (int x = 0; x <= 50; x++)
		{
			if (x == highestIndex)
			{
				fill(red);
				rect(x * w, height - randomCounts[x], w - 1, randomCounts[x]);
			}
			else
			{
				fill(green);
				rect(x * w, height - randomCounts[x], w - 1, randomCounts[x]);
			}
		}
		setWindowTitle("pseudo-random uniform distribution of numbers");
		break;
	}

	if (mouseReleased())
	{
		clear(c64blue);
		scene++;
		if (scene == 7)
			scene = 0;
	}
}

void cleanup() {}
