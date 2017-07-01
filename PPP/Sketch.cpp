#include "PPP.h"

int frames = 0;
Color bgColor;

static const int NUM_PARTICLES = 3000;

struct Particle
{
	float x;
	float y;
	float dx;
	float dy;
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
		size(1920, 1080);
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
		//background(Color(0, 0, 0, 1));
		//fill(Color(170, 170, 170, 10));
		noStroke();
		//stroke(Color(0));
		rectMode(RectMode::CORNER);
		//rect((float)0, (float)0, (float)width(), (float)height());
		int curMillis = millis();

		particles[curParticle].active = true;
		particles[curParticle].x      = mouseX();
		particles[curParticle].y      = mouseY();
		particles[curParticle].c      = Color::white;
		particles[curParticle].millis = curMillis;
		particles[curParticle].size   = 0;
		particles[curParticle].dx     = 0;
		particles[curParticle].dy     = 0;

		rectMode(RectMode::CENTER);

		for (int i = 0; i < NUM_PARTICLES; ++i)
		{
			Particle& particle = particles[(i + curParticle) % NUM_PARTICLES];
			if (particle.active)
			{
				fill(particle.c);

				rect(particle.x, particle.y, particle.size, particle.size);
				particle.c.r  = (unsigned char)(127 + 32 * sin(0.5f * particle.millis));
				particle.c.g  = (unsigned char)(60 + 16 * sin(0.25f * particle.millis));
				particle.c.b  = (unsigned char)(40 +  40 * sin(0.1f * particle.millis));
				particle.dx   += random(-2, 2);
				particle.dy   += random(-2, 2);

				/*
				if (particle.dx >  10) { particle.dx = 10; }
				if (particle.dy >  10) { particle.dy = 10; }
				if (particle.dx < -10) { particle.dx = -10; }
				if (particle.dy < -10) { particle.dy = -10; }


				particle.x += particle.dx;
				particle.y += particle.dy;
				*/
				particle.size += 0.5f;
				if (particle.size < 0)
				{
					particle.size = 0;
				}
				if (curMillis - particle.millis > 10000)
				{
					particle.active = false;
				}
			}
		}
		curParticle = (curParticle+1)% NUM_PARTICLES;
		
		for (int i = 255; i >= 0; --i)
		{
			//drawTriangle(frames / (float)(i + 1), i);
		}
		//strokeWeight(20 + 15 * sin(frames/10.f));
		fill(Color(255, 255, 255, 50));
		//triangle(mouseX() + 300 , mouseY(), mouseX() - 300, mouseY(), mouseX(), mouseY() + 300);

		++frames;

	}

	void mousePressed() override
	{
		bgColor = Color((int)random(256), (int)random(256), (int)random(256));
	}

	void mouseReleased() override
	{
		bgColor = Color((int)random(256), (int)random(256), (int)random(256));
	}

};

SET_SKETCH_CLASS(TestSketch)
