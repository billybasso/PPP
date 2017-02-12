#include "PPP.h"

int frames = 0;

void setup()
{
	size(400, 400);
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
	if (frames++ % 2 == 0)
	{
		background(Color::red);
		triangle(30, 75, 58, 20, 86, 75);
	}
	else
	{
		background(Color::blue);
	}
}


