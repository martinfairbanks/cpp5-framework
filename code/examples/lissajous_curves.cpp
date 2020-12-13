/* 	Lissajous Curves 
	A couple of examples of Lissajous curves.

	Copyright (c) 2020 Martin Fairbanks
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.

	some common inputs with odd and even numbers:
		a = 1, b = 2 (1:2) = xSpeed = .1, ySpeed = .2;
		a = 3, b = 2 (3:2)
		a = 3, b = 4 (3:4)
		a = 5, b = 4 (5:4)
		a = 5, b = 6 (5:6)
		a = 7, b = 6 (7:6)

		https://en.wikipedia.org/wiki/Lissajous_curve
*/

#include "../cpp5_framework.h"

i32 scene = 0;
f32 xAngle = 0.f;
f32 yAngle = 0.f;
f32 xSpeed = 0;
f32 ySpeed = 0;
f32 x = 0;
f32 y = 0;

struct Circles
{
	v2 pos[100];
	Color col[100];
} circles;

void setup()
{
	createCanvas(960, 540, "Lissajous Curves");
	
	clear(black);
	
	for (i32 i = 0; i < arrayCount(circles.pos); i++)
	{
		circles.pos[i].x = 0;
		circles.pos[i].y = 0;
		circles.col[i] = Color{ random(255), 255, 255, 255 };
	}

	disableDoubleBuffer();
}

void draw()
{
	//clear(black);
	colorMode(RGB);
	noStroke();
	fill(green);

	f32 xRadius = 200;
	f32 yRadius = 200;
	
	switch (scene)
	{
		case 0:
			xSpeed = .1f;
			ySpeed = .2f;
			break;
		case 1:
			xSpeed = .3f;
			ySpeed = .2f;
			break;
		case 2:
			xSpeed = .3f;
			ySpeed = .4f;
			break;
		case 3:
			xSpeed = .5f;
			ySpeed = .4f;
			break;
		case 4:
			xSpeed = .5f;
			ySpeed = .6f;
			break;
		case 5:
			xSpeed = .7f;
			ySpeed = .6f;
			break;
	}
	
	if (scene == 6)
	{
		xAngle = 0.f;
		yAngle = 0.f;
		xSpeed = .3f;
		ySpeed = .2f;
		for (f32 angle = 0; angle < TWO_PI; angle += 0.005f)
		{
			x = center.x + cos(xAngle) * xRadius;
			y = center.y + sin(yAngle) * yRadius;
			//stroke(red);
			//pixel(x, y);
			circle(x, y, 4.f);
			xAngle += xSpeed;
			yAngle += ySpeed;
		}
	}
	else if (scene == 7)
	{
		clear(black);
		colorMode(HSB);
		xSpeed = .1f;
		ySpeed = .141f;

		f32 k = 0.1f;
		for (i32 i = 0; i < arrayCount(circles.pos); i++)
		{
			circles.pos[i].x = center.x + cos(xAngle*k) * xRadius;
			circles.pos[i].y = center.y + sin(yAngle*k) * yRadius;
			k += 0.08f;
			fill(circles.col[i]);
			circle(circles.pos[i].x, circles.pos[i].y, 12.f);
		}
		
		xAngle += xSpeed*0.08f;
		yAngle += ySpeed*0.08f;
		
	}
	else
	{
		x = center.x + cos(xAngle) * xRadius;
		y = center.y + sin(yAngle) * yRadius;
		circle(x, y, 4.f);

		xAngle += xSpeed;
		yAngle += ySpeed;
	}

	if (mouseReleased())
	{
		xAngle = yAngle = 0;
		clear(black);
		scene++;
		if (scene > 7)
			scene = 0;
	}

	_sleep(10);
}

void cleanup() { }
