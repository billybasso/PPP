#pragma once

class String;
struct Color;

enum class RectMode
{
	CORNER,
	CORNERS,
	RADIUS,
	CENTER,
};

enum class JointStyle
{
	MITER,
	BEVEL,
	ROUND,
};

enum class StrokeCapStyle
{
	SQUARE,
	PROJECT,
	ROUND
};


// Constants
const float QUARTER_PI = 0.7853982f;
const float HALF_PI    = 1.5707964f;
const float PI         = 3.1415927f;
const float TWO_PI     = 6.2831855f;
const float TAU        = 6.2831855f;

//Processing API ------------------------------------------------------------
class PApplet
{
public:


	static void print(const char* text);
	static void println(const char* text);
	static void print(const String& text);
	static void println(const String& text);

	//2D Primitives
	static void triangle(float x1, float y1, float x2, float y2, float x3, float y3);
	static void quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
	static void rect(float a, float b, float c, float d);
	static void rect(float a, float b, float c, float d, float r);
	//Attributes
	void rectMode(RectMode mode);
	void strokeWeight(float weight);


	static void size(int width, int height);
	static void frameRate(float frameRate);
	static int width();
	static int height();

	//Time & Date
	static int day();
	static int hour();
	static int millis();
	static int month();
	static int second();
	static int year();

	//Color
	//Setting
	static void background(Color color);
	static void fill(Color c);
	static void stroke(Color c);
	static void noStroke();
	//Math

	//Calculation
	static int abs(int n);
	static float abs(float n);
	static int ceil(float n);
	static int constrain(int amt, int low, int high);
	static float constrain(float amt, float low, float high);
	static float dist(float x1, float y1, float x2, float y2);
	static float exp(float n);
	static int floor(float n);
	static float lerp(float start, float stop, float amt);
	static float log(float a);
	static float mag(float a, float b, float c);
	static float map(float value, float start1, float stop1, float start2, float stop2);
	static int maxt(int a, int b);
	static int max(int a, int b, int c);
	static int max(int list[], int listSize);
	static float max(float a, float b);
	static float max(float a, float b, float c);
	static float max(float list[], int listSize);
	static int mint(int a, int b);
	static int min(int a, int b, int c);
	static int min(int list[], int listSize);
	static float min(float a, float b);
	static float min(float a, float b, float c);
	static float min(float list[], int listSize);
	static float norm(float value, float start, float stop);
	static float pow(float n, float e);
	static int round(float n);
	static float sq(float n);
	static float sqrt(float n);

	//Trigonometry
	static float acos(float value);
	static float asin(float value);
	static float atan(float value);
	static float atan2(float y, float x);
	static float cos(float angle);
	static float degrees(float radians);
	static float radians(float degrees);
	static float sin(float angle);
	static float tan(float angle);

	//Random
	static float random(float high);
	static float random(float low, float high);

	static float mouseX();
	static float mouseY();

	virtual void setup() {}
	virtual void draw() {}
	virtual void mousePressed() {}
	virtual void mouseReleased() {}

};

//types

// - Color ----------------------------------------------------

struct Color
{
	Color();
	Color(unsigned char gray, unsigned char a = 255);
	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

	const Color operator *(const Color & rhs) const;
	const bool operator ==(const Color & rhs) const;
	const bool operator !=(const Color & rhs) const;

	static const Color black;
	static const Color white;
	static const Color red;
	static const Color green;
	static const Color blue;
	static const Color cyan;
	static const Color yellow;
	static const Color magenta;
	static const Color springGreen;
	static const Color chartreuse;
	static const Color orange;
	static const Color azure;
	static const Color violet;
	static const Color rose;

	unsigned char r, g, b, a;
};

// - String ----------------------------------------------------

class String
{
	char* m_data;
	int m_length;

	enum ENoInit {
		NO_INIT
	};
	String(ENoInit) {}
public:
	String() : m_data(nullptr), m_length(0) {}
	String(const char* data);
	String(const String& str, int offset, int length);
	String(const String& str);
	String(String&& str);
	String(int i);
	~String();

	char charAt(int index) const;

	int indexOf(const String& str) const;
	int indexOf(const String& str, int fromIndex) const;

	int length() const;

	String subString(int beginIndex) const;
	String subString(int beginIndex, int endIndex) const;

	String toLowerCase() const;
	String toUpperCase() const;

	String operator+(const String& str) const;
	bool operator==(const String& str) const;
	const char* operator*() const;

	friend String operator+(const char* a, const String& b);
};

String operator+(const char* a, const String& b);


extern class PApplet& getCurrentApp();

#define SET_SKETCH_CLASS( SketchClass )      \
inline PApplet& getCurrentApp()              \
{                                            \
	static SketchClass inst = SketchClass(); \
	return inst;                             \
}                                            \

