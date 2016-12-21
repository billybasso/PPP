#pragma once

class String;
struct Color;

//Processing API ------------------------------------------------------------
void print(const char* text);
void println(const char* text);
void print(const String& text);
void println(const String& text);


//Random
float random(float high);
float random(float low, float high);

void background(Color color);

void size(int width, int height);
void frameRate(float frameRate);


//these methods must be implemented for the sketch to run
void setup();
void draw();

//types

struct Color
{
	Color();
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
};

String operator+(const char* a, const String& b);



