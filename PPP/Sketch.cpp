#include "PPP.h"

int frames = 0;
Color bgColor;

static const int NUM_PARTICLES = 300;

struct Particle
{
	float x;
	float y;
	Color c;
	float size;
	bool active;
	int millis;
};

class TestSketch : public PApplet
{
	Particle particles[NUM_PARTICLES];
	int curParticle = 0;
	void setup()
	{
		size(3200, 1800);
		frameRate(60);
		for (int i = 0; i < NUM_PARTICLES; ++i)
		{
			particles[i].active = false;
			particles[i].x = 0;
			particles[i].y = 0;
			particles[i].c = Color::white;
		}
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
		background(Color::black);

		int curMillis = millis();

		particles[curParticle].active = true;
		particles[curParticle].x      = mouseX();
		particles[curParticle].y      = mouseY();
		particles[curParticle].c      = Color::white;
		particles[curParticle].millis = curMillis;
		particles[curParticle].size   = 100;

		rectMode(CENTER);

		for (int i = 0; i < NUM_PARTICLES; ++i)
		{
			Particle& particle = particles[(i + curParticle) % NUM_PARTICLES];
			if (particle.active)
			{
				fill(particle.c);
				rect(particle.x, particle.y, particle.size, particle.size);
				particle.c.r  = maxt(particle.c.r - 2, 0);
				particle.c.g  = maxt(particle.c.g - 1, 0);
				particle.c.b  = maxt(particle.c.b - 1, 0);
				particle.x += random(-2, 2);
				particle.y += random(-2, 2);
				particle.size -= 0.5;
				if (curMillis - particle.millis > 6000)
				{
					particle.active = false;
				}
			}
		}
		curParticle = (curParticle+1)% NUM_PARTICLES;
		
		//for (int i = 255; i >= 0; --i)
		//{
		//	drawTriangle(frames / (float)(i + 1), i);
		//}
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
