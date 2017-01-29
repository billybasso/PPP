#include "PPP.h"

void setup()
{
	size(400, 400);
	frameRate(4);

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
	println("year: " + String(year()));
	println("month: " + String(month()));
	println("day: " + String(day()));
}

void draw()
{
	int randomNumber = (int)random(2);
	if (randomNumber >= 1)
	{
		background(Color::orange);
		triangle(30, 75, 58, 20, 86, 75);
	}
	else
	{
		background(Color::azure);
	}
}


