#include "PPP.h"

void setup()
{
	size(400, 400);
	frameRate(1);

	String str = "Cool String";

	String cool = String(str, 0, 4);

	println(str);
	println(cool);

	String coolCoolString = cool + str;
	println(coolCoolString);
}

void draw()
{
	println("drawing...");

	int randomNumber = (int)random(2);
	if (randomNumber >= 1)
	{
		background(Color::orange);
	}
	else
	{
		background(Color::azure);
	}
}