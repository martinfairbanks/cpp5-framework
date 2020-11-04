#include "../framework/cpp5_git.h"
//#include "../framework/creativeframework_sdl_ogl.h"

f32 angle = 0;
f32 zPos = 0;
u32 texture = 0;
i32 myModel = 0;

void setup()
{
	createCanvas(960, 540);
	texture = loadTexture("data/pix/karate.bmp");
	//myModel = loadModel("data/3d/cube.obj");
	myModel = loadModel("data/3d/monkey.obj");
	
	//disableDoubleBuffer();
	clear(112, 112, 222);
	clear(0);
}

void draw()
{
	loadIdentity();
#if 1
	colorMode(RGB);
	clear(c64blue);
	strokeWeight(4);
	stroke(blue);
	fill(green);
	line(150, 50, 250, 50);
	circle(300, 50, 50);
	fill(red);
	circle(350, 50, 10);
	rect(500, 50, 50, 50);
	stroke(green);
	fill(magenta);
	ellipse(600, 200, 80, 100);
	noFill();
	circle(100, 400, 50);
	rect(100, 500, 50, 50);
	fill(blue);
	stroke(red);
	noStroke();
	quad(30, 30, 100, 30, 80, 100, 50, 100);
	strokeWeight(10);
	stroke(yellow);
	line(290, 250, 450, 150);
#if 1
	enableFog(-5.0f, 5.0f, 0.4f, 0.4f, 0.4f, 1.0f);
	sprite(texture, 200, 100, 200, 200);
	disableFog();


	set3dProjection();
	stroke(black);
	strokeWeight(2);
	fill(255, 0, 150);
	lights();
	translate((f32)mouseX, -(f32)mouseY, -200.f);
	//translate(absoluteValue((f32)(mouseX - width/2)), absoluteValue((f32)(mouseY-height) - height/2));
	//rectMode(CENTER);
	rotateX(-angle);
	rotateY(-angle * 0.3f);
	rotateZ(-angle * 0.8f);

	//box(50,50,20);
	torus(50, 10);

	angle += 0.1f;

	rectMode(CENTER);
	fill(blue);
	rect(0, 0, 150, 100);
	fill(0, 0, 150);
	translate(0, 0, -1.f);
	circle(0, 0, 50);
	fill(red);
	box(10, 10, 50);

	rotateX(angle);
	rotateY(angle * 2.3f);
	rotateZ(angle * 1.2f);

	stroke(pink);
	fill(black);
	loadIdentity();
	pushMatrix();
	translate(0.0f, 0.0f, -10.0f);
	rotateY(angle);
	rotateX(angle);
	model(myModel);
	popMatrix();

	loadIdentity();
	translate(0.0f, 0.0f, zPos);
	rotateX(angle);

	//draw pyramid
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);		//top - front
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);		//left
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);		//right

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);		//top - right
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);		//left 
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);		//right

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);		//top - back
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);		//left
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);	//right

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);		//top - left
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);	//left
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);		//right 
	glEnd();

	zPos = 4.0f * ((f32)cos(angle / 20.0f) - 1.5f);
#endif
#endif

	set2dProjection();

#if 1	
	
	static f32 a = 0;
	static b32 done = false;
	static f32 x = 120;
	static i32 col = 0;
	
	colorMode(HSB);
	noStroke();
	fill(col, 255, 255);
	f32 y = (f32)(height / 2);
    
	if (!done)
		circle(x, y, 5.f+sin(a*4.f)*150.0f);
	else {
		x = 120;
		a = 0;
		done = false;
	}
	a = a + 0.001f;
	x++;
	col = (col + 1) % 255;
	//when a is 6.28 we have completed one full rotation around the center
	if (a > TWO_PI-0.1) {
		a = 0;
		done = true;
	}
#endif
	colorMode(RGB);
	stroke(red);
	point(width / 2, height / 2);
	point(width / 2 + 1, height / 2);
	point(width / 2+1, height / 2+1);
	point(width / 2, height / 2+1);
	text(100, 100, "Hello");
}

void shutdown() { }
