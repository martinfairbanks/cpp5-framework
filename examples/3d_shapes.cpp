/* 	3D Shapes 
	by Martin Fairbanks

	Demonstration of different 3D shapes.
*/

#include "../../framework/creative_framework.h"
f32 angle = 0.f;

void setup()
{
	createCanvas(960, 540, false, "3D Shapes");
	//set3dProjection(windowWidth,windowHeight, 120.0f, 0.1f, 500.0f);
	set3dProjection();
}

void updateAndDraw()
{
	clear(c64blue);
	glLoadIdentity();
	translate(10.f, -15.f, -80.f);
	//draw cube
	pushMatrix();
		fill(magenta);
		translate(-55.f, 0.f, 0.f);
		rotateX(angle);
		rotateY(angle);
		rotateZ(angle);
		cube(8);
	popMatrix();

	//draw plane
	pushMatrix();
		fill(blue);
		translate(-30.f, 0.f, 0.f);
		rotateX(angle);
		rotateY(angle);
		rotateZ(angle);
		plane(20, 20);
	popMatrix();

	//draw sphere
	pushMatrix();
		fill(pink);
		translate(0.f, 0.0f, 0.f);
		rotateX(angle);
		rotateY(angle);
		rotateZ(angle);
		noFill();
		sphere(10);
	popMatrix();

	//draw torus
	pushMatrix();
		fill(green);
		translate(40.f, 0.0f, 0.f);
		rotateX(angle);
		rotateY(angle);
		rotateZ(angle);
		noFill();
		torus(12, 6);
	popMatrix();

	//draw box
	pushMatrix();
		fill(c64cyan);
		translate(-55.f, 35.f, 0.f);
		rotateX(angle);
		rotateY(angle);
		rotateZ(angle);
		box(5, 10, 15);
	popMatrix();

	//draw cone
	pushMatrix();
		fill(purple);
		translate(-25.f, 35.f, 0.f);
		rotateX(angle);
		//rotateY(angle);
		rotateZ(angle);
		cone(10, 20);
	popMatrix();

	//draw cylinder
	pushMatrix();
		fill(yellow);
		translate(0.f, 35.f, 0.f);
		rotateX(angle);
		rotateY(angle);
		rotateZ(angle);
		cylinder(10, 20);
	popMatrix();

	//draw pyramid
	pushMatrix();
		fill(red);
		noFill();
		translate(40.f, 35.f, 0.f);
		rotateX(angle);
		rotateY(angle);
		rotateZ(angle);
		pyramid(10,15);
	popMatrix();

	angle += deltaTime*20;
}

void shutdown() { }