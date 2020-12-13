#include "../cpp5_framework.h"
Color col;

struct Attractor
{
	v2 position = { center.x, center.y };
	i32 mass = 20;
	i32 G = 1;	//gravitational constant
	v2 dragOffset = { 0, 0 };
	b32 dragging = false;
	b32 rollover = false;

	void display()
	{
		//ellipseMode(CENTER);
		//strokeWeight(4);
		if (dragging)
			fill(red);
		else if (rollover)
			fill(blue);
		else 
			fill(green);

		circle(position.x, position.y, (f32)(mass * 2));
	};

	void handlePress(i32 mx, i32 my)
	{
		f32 d = dist(v2((f32)mx, (f32)my), position);
		if (d < mass * 2)
		{
			dragging = true;
			dragOffset.x = position.x - mx;
			dragOffset.y = position.y - my;
		}
	};

	void handleHover(i32 mx, i32 my)
	{
		f32 d = dist(v2((f32)mx, (f32)my), position);
		if (d < mass * 2)
			rollover = true;
		else
			rollover = false;
	};

	void stopDragging()
	{
		dragging = false;
	};

	void handleDrag(i32 mx, i32 my)
	{
		if (dragging)
		{
			position.x = mx + dragOffset.x;
			position.y = my + dragOffset.y;
		}
	};
} attractor;

void setup()
{
	createCanvas(960, 540, "input");
}

void draw()
{
	clear(90, 80, 140);
	stroke(0, 255, 0);
	fill(col);
	
	if (mousePressed(MOUSE_LEFT))
	{
		fill(blue);
		rect(10, 10, 100, 100);
	}

	if (mousePressed(MOUSE_MIDDLE))
	{
		fill(red);
		rect(10*10, 10, 100, 100);
	}

	if (mousePressed(MOUSE_RIGHT))
	{
		fill(purple);
		rect(10 * 20, 10, 100, 100);
	}

	if (mouseReleased(MOUSE_LEFT))
	{
		col = { random(255), random(255), random(255), 255 };
	}
	
	if (mouseClicked(MOUSE_LEFT))
	{
		col = { random(255), random(255), random(255), 255 };
	}
	
	if (keyDown(VK_SPACE))
	{
		col = { random(255), random(255), random(255),255 };
	}

	if (keyUp(VK_UP))
		col = { random(255), random(255), random(255),255 };

	if (keyHit(0x41)) //a
	{
		col = { random(255), random(255), random(255), 255 };
	}	

	if (mouseMoved())
	{
		attractor.handleHover(mouseX, mouseY);
	}

	if (mousePressed(MOUSE_LEFT))
	{
		attractor.handlePress(mouseX, mouseY);
	}

	if (mouseDragged)
	{
		attractor.handleHover(mouseX, mouseY);
		attractor.handleDrag(mouseX, mouseY);
	}

	if (mouseReleased(MOUSE_LEFT))
	{
		attractor.stopDragging();
	}
	
	attractor.display();

	fill(col);
	circle(mouseX, mouseY, 10);

}

void cleanup() { }
