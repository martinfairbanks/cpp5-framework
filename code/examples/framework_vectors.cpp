/*	Vectors

	Euclidean vector (geometric vector) is an entity that has both magnitude and direction.
			A -----> B
		
	In a Cartesian coordinate system you can think of a vector as a x and y point in space or an arrow with a length and an angle.
	You can use vectors to represent different things:
		
	Position vector
	A position can be a vector representing the difference between position and origin.
	Therefore you can think of it as the distance (difference) between two points.
		magnitude -> the distance between two points, the length of the vector
		direction -> the angle that the vector is pointing in
		
	Velocity vector
	magnitude -> represent speed, the faster the speed the longer the length
	direction -> the angle (heading) in which the object is moving

	This declares a vector that are 10 units in the x-direction and 5 units in the y-direction,
	you can also think of it as an x- and y-coordinate:
		v2 vec = v2(10.f, 5.f);

	Copyright (c) 2020 Martin Fairbanks
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.	
*/
#define DEVELOPER 1
#define DEBUGGER_MSVC 1
#include "../cpp5_framework.h"

void setup()
{
	createCanvas(960, 540, "vectors");
}

void draw()
{
	v2 mouse = { (f32)mouseX, (f32)mouseY };
	static v2 position = center;
	static v2 velocity = { 1.f, 1.f };

	v2 acceleration = { 0.15f, 0.02f };
	f32 magnitude;
	f32 radius = 70.f;
	//i32 topspeed = 10;

	clear(cornflowerblue);
	stroke(blue);
	fill(magenta);

	static i32 scene = 0;
	switch (scene)
	{
		case 0:
			setWindowTitle("vector addition");

			position += velocity;

			if ((position.x + radius > width) || (position.x- radius < 0))
				velocity.x *= -1;
		
			if ((position.y + radius > height) || (position.y - radius  < 0))
				velocity.y *= -1;

			circle(position.x, position.y, radius);
			break;

		case 1:
			setWindowTitle("vector subtraction");
			mouse -= center;
			line(center.x, center.y, center.x+mouse.x, center.y+mouse.y);
			break;

		case 2:
			setWindowTitle("vector scaling - vector multiplication by a scalar number");
			mouse -= center;
			//vector multiplication to make the vector half it's size
			mouse *= 0.5f;
			line(center.x, center.y, center.x + mouse.x, center.y + mouse.y);
			break;

		case 3:
			setWindowTitle("vector magnitude (length)");
			mouse -= center;
			//calculate length (magnitude) of the vector, square root of x^2 + y^2
			//The Pythagorean Theorem
			//magnitude = sqrt(mouse.x*mouse.x + mouse.y*mouse.y);
			magnitude = mouse.length();

			stroke(blue);
			fill(orange);
			circle(width / 2.f, height / 2.f, magnitude);
			line(center.x, center.y, center.x + mouse.x, center.y + mouse.y);
			rect(50, (i32)magnitude, 20, height);
			rect(width - 50, 0, 20, (i32)magnitude);
			break;

		case 4:
			setWindowTitle("normalize vector, sets its length to 1");
			mouse -= center;
			//normalize the vector, sets its length to 1, but it's pointing in the same direction
			//to normalize a vector - multiply it by the inverse of its length
			//or divide each component by its magnitude
	#if 1
			magnitude = sqrt(mouse.x*mouse.x + mouse.y * mouse.y);
			mouse.x = mouse.x / magnitude;
			mouse.y = mouse.y / magnitude;
			magnitude = sqrt(mouse.x*mouse.x + mouse.y * mouse.y);
			//printLog("%f", mouse.x);
			//printLog("%f", mouse.y);
			//printLog("%f", magnitude);
	#elif
			mouse.normalize();
			magnitude = mouse.length();
			printLog("%f", mouse.x);
			printLog("%f", mouse.y);
			printLog("%f", magnitude);
	#endif
			//multiply length by 140 just to show it on screen
			//because it's normalized it will always have a length of 140
			mouse *= 140;
			line(center.x, center.y, center.x + mouse.x, center.y + mouse.y);
			break;

	}

	if (mouseReleased())
	{
		scene++;
		position = center;
		if (scene == 5)
			scene = 0;
	}
}

void cleanup() { }
