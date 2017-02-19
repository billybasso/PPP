#include "PPP.h"

int frames = 0;
Color bgColor;

class TestSketch : public PApplet
{
	void setup()
	{
		size(700, 700);
		frameRate(60);
	}

	void drawTriangle(float frames, int i)
	{
		fill((i *4)%255);

		float speed = 0.01f;

		float x1 = width()  * 0.5f;
		float y1 = height() * 0.5f;
		float x2 = width()  * (1 + cos(0.1f + frames * TAU * speed)) * 0.5f;
		float y2 = height() * (1 + sin(0.1f + frames * TAU * speed)) * 0.5f;
		float x3 = width()  * (1 + cos(0.2f + frames * TAU * speed)) * 0.5f;
		float y3 = height() * (1 + sin(0.2f + frames * TAU * speed)) * 0.5f;
		triangle(x1, y1, x2, y2, x3, y3);
	}

	void draw()
	{
		//background(Color::white);
		//for (int i = 255; i >= 0; --i)
		//{
		//	drawTriangle(frames / (float)(i + 1), i);
		//}
		int w = width();
		int h = height();
		rectMode(CORNERS);
		fill(Color::azure);
		rect(w*0.25f, h*0.25f, w*0.75f, h*0.75f);
		++frames;

	}

	void mousePressed() override
	{
		bgColor = Color(random(256), random(256), random(256));
	}

	void mouseReleased() override
	{
		bgColor = Color(random(256), random(256), random(256));
	}

};

SET_SKETCH_CLASS(TestSketch)
