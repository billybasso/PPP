#include "PPP.h"

int frames = 0;

void setup()
{
	size(3200, 1800);
	frameRate(60);

	String str = "Cool String";

	String cool = String(str, 0, 4);

	println(str);
	println(cool);

	String mixed = "HeY ThEre!";
	String lower = mixed.toLowerCase();
	String upper = lower.toUpperCase();
	println("upper: " + upper);
	String coolCoolString = cool + str;
	println(coolCoolString);

	String yearStr = year();
	String monthStr = month();
	String dayStr = day();
	println("year: " + yearStr);
	println("month: " + monthStr);
	println("day: " + dayStr);
}

void draw()
{
	//background(Color::black);
	float x = random(-1, 1);
	float y = random(-1, 1);
	fill(Color(x * 256, y * 256, 255 - 255 * x));
	triangle(x + .3f, y + 0, x -.3f, y + 0, x + 0, y + .3f);
	
}


