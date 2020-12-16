/* 	Image and model
	Loading and drawing an image and a 3d model.
	
	Copyright (c) 2020 Martin Fairbanks
	This example has been created using the cpp5 framework.
	Licensing information can be found in the cpp5_framework.h file.
*/


#include "../cpp5_framework.h"
u32 texture = 0;
i32 myModel = 0;
f32 angle = 0.f;

void setup()
{
	createCanvas(960, 540, "sprites");
	texture = loadTexture("data/pix/karate.bmp");
	myModel = loadModel("data/3d/monkey.obj");
}

void draw()
{
	clear(cornflowerblue);
	set2dProjection();
	sprite(texture, 200, 100, 200, 200);
	enableFog(-5.0f, 5.0f, 0.4f, 0.4f, 0.4f, 1.0f);
	sprite(texture, 600, 100, 200, 200);
	disableFog();

	set3dProjection();
	stroke(blue);
	lights();
	loadIdentity();
	pushMatrix();
		translate(0.0f, 0.0f, -3.0f);
		rotateY(angle);
		rotateX(angle);
		model(myModel);
	popMatrix();
	angle += deltaTime * 20;
}

void cleanup() { }