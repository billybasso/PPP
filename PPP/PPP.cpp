#include "PPP.h"

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include <vector>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

#include <stdlib.h>
#include <assert.h>

#pragma comment(lib, "d3d11")

static const int NUM_VERTS = 30000;

static const int DEFAULT_WINDOW_WIDTH  = 100;
static const int DEFAULT_WINDOW_HEIGHT = 100;
static const float DEFAULT_FRAME_RATE  = 60;

struct Vertex_2DPosColor
{
	XMFLOAT2 pos;
	Color color;
};

struct WinApplicationState
{
	HINSTANCE hInstance;
	HWND hWindow; //window handle
	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dImmediateContext;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11RasterizerState* rasterizerState;
	IDXGISwapChain* swapChain;
	int width     = DEFAULT_WINDOW_WIDTH;
	int height    = DEFAULT_WINDOW_HEIGHT;
	float frameRate = DEFAULT_FRAME_RATE;
	__int64 programStartCount;
	__int64 clockFrequency;
	__int64 totalClockCount;

	float mouseX = 0;
	float mouseY = 0;
	ID3D11Buffer* vertBuffer;
	vector<Vertex_2DPosColor> verts;

	//processing state
	//style
	Color fillColor = { 255, 255, 255, 255 };
	RectMode rectMode = CORNER;

};

static WinApplicationState gState;


void Draw()
{
	gState.verts.clear();
	getCurrentApp().draw(); //call out to game code

	//gState.d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gState.d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	
	gState.d3dImmediateContext->Map(gState.vertBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	size_t vertSize = sizeof(Vertex_2DPosColor);
	size_t byteSize = vertSize * (UINT)gState.verts.size();
	memcpy(mappedResource.pData, gState.verts.data(), byteSize);
	gState.d3dImmediateContext->Unmap(gState.vertBuffer, 0);

	gState.d3dImmediateContext->RSSetState(gState.rasterizerState);
	gState.d3dImmediateContext->Draw((UINT)gState.verts.size(), 0);
	gState.swapChain->Present(0, 0);

	gState.d3dImmediateContext->RSSetState(0); //restore the default state.

}

LRESULT CALLBACK WndProc(
	HWND   hWnd,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		getCurrentApp().mousePressed();
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		getCurrentApp().mouseReleased();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}


int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	gState = WinApplicationState();
	getCurrentApp().setup(); //call out to game code

	gState.hInstance = hInstance;

	static TCHAR szWindowClass[] = TEXT("win32app");
	static TCHAR szTitle[]       = TEXT("Sketch");

	WNDCLASSEX wcex;
	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInstance;
	wcex.hIcon         = NULL;
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm       = NULL;

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			TEXT("Call to RegisterClassEx failed!"),
			TEXT("Win32 Guided Tour"),
			NULL
		);

		return 1;
	}

	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	RECT desiredSize = { 0,0,gState.width,gState.height };
	BOOL hasMenu = false;
	AdjustWindowRect(&desiredSize, style, false);
	gState.hWindow = CreateWindow(
		szWindowClass,
		szTitle,
		style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		desiredSize.right - desiredSize.left, desiredSize.bottom - desiredSize.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!gState.hWindow)
	{
		MessageBox(NULL,
			TEXT("Call to Create Window failed!"),
			TEXT("Sketch"),
			NULL);

		return 1;
	}
	ShowWindow(gState.hWindow, nShowCmd);
	UpdateWindow(gState.hWindow);

	//init d3d
	{
		UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevel;
		ID3D11Device* d3dDevice;
		ID3D11DeviceContext* d3dImmediateContext;
		HRESULT hr = D3D11CreateDevice(
			0,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			createDeviceFlags,
			0, 0,
			D3D11_SDK_VERSION,
			&d3dDevice,
			&featureLevel,
			&d3dImmediateContext);

		if (FAILED(hr))
		{
			MessageBox(0, TEXT("D3D11CreateDevice Failed"), 0, 0);
			return 1;
		}
		if (featureLevel != D3D_FEATURE_LEVEL_11_0)
		{
			MessageBox(0, TEXT("Direct3D Feature Level 11 unsupported"), 0, 0);
			return 1;
		}

		{
			UINT m4xMsaaQuality;
			d3dDevice->CheckMultisampleQualityLevels(
				DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);
			assert(m4xMsaaQuality > 0);
		}

		//create swap chain
		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width                   = gState.width;
		sd.BufferDesc.Height                  = gState.height;
		sd.BufferDesc.RefreshRate.Numerator   = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SampleDesc.Count                   = 1; //don't use msaa
		sd.SampleDesc.Quality                 = 0;
		sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount                        = 1;
		sd.OutputWindow                       = gState.hWindow;
		sd.Windowed                           = true;
		sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags                              = 0;

		IDXGIDevice* dxgiDevice = 0;
		d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
		IDXGIAdapter* dxgiAdapter = 0;
		dxgiDevice->GetAdapter(&dxgiAdapter);
		IDXGIFactory* dxgiFactory = 0;
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

		IDXGISwapChain* swapChain;
		dxgiFactory->CreateSwapChain(d3dDevice, &sd, &swapChain);

		dxgiDevice->Release();
		dxgiAdapter->Release();
		dxgiFactory->Release();

		//create the render target view
		ID3D11RenderTargetView* mRenderTargetView;
		ID3D11Texture2D* backBuffer;
		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
		d3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView);
		backBuffer->Release();

		//create the depth/stencil buffer and view
		//D3D11_TEXTURE2D_DESC depthStencilDesc;
		//depthStencilDesc.Width              = DEFAULT_WINDOW_WIDTH;
		//depthStencilDesc.Height             = DEFAULT_WINDOW_HEIGHT;
		//depthStencilDesc.MipLevels          = 1;
		//depthStencilDesc.ArraySize          = 1;
		//depthStencilDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
		//depthStencilDesc.SampleDesc.Count   = 1; //don't use msaa
		//depthStencilDesc.SampleDesc.Quality = 0;
		//depthStencilDesc.Usage              = D3D11_USAGE_DEFAULT;
		//depthStencilDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
		//depthStencilDesc.CPUAccessFlags     = 0;
		//depthStencilDesc.MiscFlags          = 0;

		//ID3D11Texture2D* mDepthStencilBuffer;
		//ID3D11DepthStencilView* mDepthStencilView;

		//d3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer);
		//d3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);

		d3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, NULL);// mDepthStencilView);

		//Set the viewport
		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.Width    = (FLOAT)gState.width;
		vp.Height   = (FLOAT)gState.height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 0.0f;
		d3dImmediateContext->RSSetViewports(1, &vp);

		gState.d3dDevice           = d3dDevice;
		gState.d3dImmediateContext = d3dImmediateContext;
		gState.renderTargetView    = mRenderTargetView;
		gState.swapChain           = swapChain;

		//todo -- set shared params
#if _DEBUG
		TCHAR vertexShaderPath[] = TEXT("../x64/Debug/VertexShader_Basic2D.cso");
		TCHAR pixelShaderPath[] = TEXT("../x64/Debug/PixelShader_Basic2D.cso");
#else
		TCHAR vertexShaderPath[] = TEXT("../x64/Release/VertexShader_Basic2D.cso");
		TCHAR pixelShaderPath[]  = TEXT("../x64/Release/PixelShader_Basic2D.cso");
#endif

		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ID3D11VertexShader* vertexShader;
		ID3D11InputLayout* inputLayout;
		{
			static const int MAX_FILE_SIZE = 1024 * 32;
			BYTE buffer[MAX_FILE_SIZE];

			HANDLE fileHandle = CreateFile(vertexShaderPath,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				__debugbreak();
			}
			DWORD bytesRead = 0;
			BOOL readResult = ReadFile(fileHandle, buffer, MAX_FILE_SIZE, &bytesRead, NULL);
			if (readResult == false)
			{
				__debugbreak();
			}
			CloseHandle(fileHandle);

			gState.d3dDevice->CreateVertexShader( buffer, bytesRead, nullptr, &vertexShader );

			gState.d3dDevice->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				buffer,
				bytesRead,
				&inputLayout
			);
		}

		ID3D11PixelShader* pixelShader;
		{
			static const int MAX_FILE_SIZE = 1024 * 32;
			BYTE buffer[MAX_FILE_SIZE];
			HANDLE fileHandle = CreateFile(pixelShaderPath,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				__debugbreak();
			}
			DWORD bytesRead = 0;
			BOOL readResult = ReadFile(fileHandle, buffer, MAX_FILE_SIZE, &bytesRead, NULL);
			if (readResult == false)
			{
				__debugbreak();
			}
			CloseHandle(fileHandle);
			gState.d3dDevice->CreatePixelShader(buffer, bytesRead, nullptr, &pixelShader);
		}

		gState.d3dImmediateContext->IASetInputLayout(inputLayout);
		gState.d3dImmediateContext->VSSetShader(vertexShader, nullptr, 0);
		gState.d3dImmediateContext->PSSetShader(pixelShader, nullptr, 0);
		gState.d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_BUFFER_DESC vertBufferDesc;
		vertBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		vertBufferDesc.ByteWidth           = sizeof(Vertex_2DPosColor)* NUM_VERTS;
		vertBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
		vertBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		vertBufferDesc.MiscFlags           = 0;
		vertBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertInitData;
		gState.verts.reserve(NUM_VERTS);
		vertInitData.pSysMem = gState.verts.data();

		gState.d3dDevice->CreateBuffer(&vertBufferDesc, &vertInitData, &gState.vertBuffer);

		UINT stride = sizeof(Vertex_2DPosColor);
		UINT offset = 0;
		gState.d3dImmediateContext->IASetVertexBuffers(0, 1, &gState.vertBuffer, &stride, &offset);

		/*
		//Create index buffer
		UINT indices[6] = {
			0, 1, 2,
			0, 2, 3
		};

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth           = sizeof(UINT) * 6;
		indexBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags      = 0;
		indexBufferDesc.MiscFlags           = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexInitData;
		indexInitData.pSysMem = indices;

		ID3D11Buffer* indexBuffer;
		gState.d3dDevice->CreateBuffer(&indexBufferDesc, &indexInitData, &indexBuffer);

		gState.d3dImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		*/


		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerDesc.FillMode              = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode              = D3D11_CULL_NONE; //todo -- this could maybe be changed
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthClipEnable       = false; //the book set this to true
		rasterizerDesc.MultisampleEnable     = false;
		rasterizerDesc.AntialiasedLineEnable = false;
		gState.d3dDevice->CreateRasterizerState(&rasterizerDesc, &gState.rasterizerState);
	}
	

	QueryPerformanceFrequency((LARGE_INTEGER*)&gState.clockFrequency);
	double period = 1.0 / gState.clockFrequency;

	__int64 prevCount;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevCount);
	gState.programStartCount = prevCount;
	__int64 counts = prevCount;
	const float framePeriod = 1.0f / gState.frameRate;
	double dt = 0;

	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&counts);
			gState.totalClockCount = counts;

			dt += (counts - prevCount) * period;
			if (dt > framePeriod)
			{
				POINT p;
				GetCursorPos(&p);
				ScreenToClient(gState.hWindow, &p);
				gState.mouseX = (float)p.x;
				gState.mouseY = (float)p.y;
				RECT rect;
				GetClientRect(gState.hWindow, &rect);
				PApplet::println(String("client rect ") + rect.right + ", " + rect.bottom);
				Draw();
				dt -= framePeriod;
			}
			prevCount = counts;
		}
	}

	return (int)msg.wParam;
}



//Color --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Color::Color()
	: r(0), b(0), g(0), a(255) {}

Color::Color(unsigned char gray, unsigned char a)
	: Color(gray, gray, gray, a) {}

Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	: r(r), g(g), b(b), a(a) {}


const Color Color::operator *(const Color & rhs) const
{
	return Color(r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a);
}

const bool Color::operator ==(const Color & rhs) const
{
	return r == rhs.r
		&& g == rhs.g
		&& b == rhs.b
		&& a == rhs.a;
}

const bool Color::operator !=(const Color & rhs) const
{
	return r != rhs.r
		|| g != rhs.g
		|| b != rhs.b
		|| a != rhs.a;
}

const Color Color::black = Color(0, 0, 0);
const Color Color::white = Color(255, 255, 255);
const Color Color::red = Color(255, 0, 0);
const Color Color::green = Color(0, 255, 0);
const Color Color::blue = Color(0, 0, 255);
const Color Color::cyan = Color(0, 255, 255);
const Color Color::yellow = Color(255, 255, 0);
const Color Color::magenta = Color(255, 0, 255);
const Color Color::orange = Color(255, 127, 0);
const Color Color::azure = Color(0, 127, 255);
const Color Color::violet = Color(127, 0, 255);
const Color Color::rose = Color(255, 0, 127);
const Color Color::chartreuse = Color(127, 255, 0);
const Color Color::springGreen = Color(0, 255, 127);

//string -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

String::String(const char* data)
{
	m_length = (int)strlen(data);
	m_data = new char[m_length + 1];
	memcpy(m_data, data, m_length + 1);
}
String::String(const String& str, int offset, int length)
{
	m_length = length;
	m_data = new char[length + 1];
	memcpy(m_data, str.m_data + offset, length);
	m_data[m_length] = '\0';
}
String::String(const String & str)
{
	m_length = str.m_length;
	if (m_length > 0)
	{
		m_data = new char[m_length + 1];
		memcpy(m_data, str.m_data, m_length + 1);
	}
	
}
String::String(String && str)
{
	m_data   = str.m_data;
	m_length = str.m_length;

	str.m_data = nullptr;
	str.m_length = 0;
}
String::String(int i)
{
	static const int MAX = 12;
	m_data = new char[MAX];
	_itoa_s(i, m_data, MAX, 10);
	m_length = (int)strlen(m_data);
}
String::~String()
{
	if (m_data)
	{
		delete[] m_data;
	}
}
char String::charAt(int index) const
{
	assert(index >= 0);
	assert(index < m_length);
	return m_data[index];
}
int String::length() const
{
	return m_length;
}
const char* String::operator*() const
{
	return m_data;
}
String String::subString(int beginIndex) const
{
	String newString(m_data + beginIndex);
	return newString;
}
String String::subString(int beginIndex, int endIndex) const
{
	String newString(NO_INIT);
	newString.m_length = endIndex - beginIndex;
	if (newString.m_length > 0)
	{
		newString.m_data = new char[newString.m_length + 1];
		memcpy(newString.m_data, m_data + beginIndex, newString.m_length);
		newString.m_data[newString.m_length] = '\0';
	}
	else
	{
		newString.m_data = nullptr;
	}
	return newString;
}
String String::toLowerCase() const
{
	String lower = *this;
	for (int i = 0; lower.m_data[i] != '\0'; ++i)
	{
		lower.m_data[i] = tolower(lower.m_data[i]);
	}
	return lower;
}
String String::toUpperCase() const
{
	String upper= *this;
	for (int i = 0; upper.m_data[i] != '\0'; ++i)
	{
		upper.m_data[i] = toupper(upper.m_data[i]);
	}
	return upper;
}
String String::operator+(const String& str) const
{
	String newString(NO_INIT);
	newString.m_length = this->m_length + str.m_length;
	newString.m_data   = new char[newString.m_length + 1];
	memcpy(newString.m_data, this->m_data, this->m_length);
	memcpy(newString.m_data + this->m_length, str.m_data, str.m_length);
	newString.m_data[newString.m_length] = '\0';
	return newString;

}
bool String::operator==(const String& str) const
{
	if (m_length != str.m_length)
	{
		return false;
	}
	for (int i = 0; i < m_length; ++i)
	{
		if (m_data[i] != str.m_data[i])
		{
			return false;
		}

	}

	return true;
}
String operator+(const char* a, const String& b)
{
	String newString(String::NO_INIT);
	int aLen = (int)strlen(a);
	newString.m_length = aLen + b.m_length;
	newString.m_data = new char[newString.m_length + 1];
	memcpy(newString.m_data, a, aLen);
	memcpy(newString.m_data + aLen, b.m_data, b.m_length);
	newString.m_data[newString.m_length] = '\0';
	return newString;
}

// Processing functions ----------------------------------------------------------------------------------------------------------------------------------------------------------------------

void PApplet::print(const char* text)
{
	OutputDebugStringA(text);
}

void PApplet::println(const char* text)
{
	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

void PApplet::println(const String& text)
{
	println(*text);
}

void PApplet::size(int width, int height)
{
	gState.width = width;
	gState.height = height;

	//todo recreate render target
}

void PApplet::frameRate(float frameRate)
{
	gState.frameRate = frameRate;
}

int PApplet::width()
{
	return gState.width;
}

int PApplet::height()
{
	return gState.height;
}

int PApplet::day()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wDay;
}

int PApplet::hour()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wHour;
}

int PApplet::millis()
{
	return  (int)(1000.0f * (gState.totalClockCount - gState.programStartCount )/ (float)gState.clockFrequency);
}

int PApplet::month()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wMonth;
}

int PApplet::second()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wSecond;
}

int PApplet::year()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wYear;
}

void PApplet::fill(Color c)
{
	gState.fillColor = c;
}



static inline float pixelToViewportX(float x)
{
	return (x / gState.width) * 2 - 1;
}

static inline float pixelToViewportY(float y)
{
	return (1 - (y / gState.height)) * 2 - 1;
}

static inline XMFLOAT2 pixelToViewport(float x, float y)
{
	return XMFLOAT2( pixelToViewportX(x), pixelToViewportY(y) );
}

void PApplet::triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	Vertex_2DPosColor v0;
	Vertex_2DPosColor v1;
	Vertex_2DPosColor v2;
	v0.pos   = pixelToViewport(x1, y1);
	v1.pos   = pixelToViewport(x2, y2);
	v2.pos   = pixelToViewport(x3, y3);
	v0.color = gState.fillColor;
	v1.color = gState.fillColor;
	v2.color = gState.fillColor;
	gState.verts.push_back(v0);
	gState.verts.push_back(v1);
	gState.verts.push_back(v2);
}

void PApplet::quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
	triangle(x1, y1, x2, y2, x3, y3);
	triangle(x3, y3, x4, y4, x1, y1);
}

void PApplet::rect(float a, float b, float c, float d)
{
	switch (gState.rectMode)
	{
	case CORNER:
		quad(a, b, a + c, b, a + c, b + d, a, b + d);
		break;
	case CORNERS:
		quad(a, b, c, b, c, d, a, d);
		break;
	case RADIUS:
		quad(a - c, b - d, a + c, b - d, a + c, b + d, a - c, b + d);
		break;
	case CENTER:
		quad(a - c/2, b - d/2, a + c/2, b - d/2, a + c/2, b + d/2, a - c/2, b + d/2);
		break;
	}
}

void PApplet::rect(float a, float b, float c, float d, float r)
{
	float x1, y1, x2, y2, x3, y3, x4, y4;
	switch (gState.rectMode)
	{
	case CORNER:
		x1 = a + r;
		y1 = b + r;
		x2 = a + c - r;
		y2 = b + r;
		x3 = a + c - r;
		y3 = b + d - r;
		x4 = a + r;
		y4 = b + d - r;
		break;
	case CORNERS:
		x1 = a + r;
		y1 = b + r;
		x2 = c - r;
		y2 = b + r;
		x3 = c - r;
		y3 = d - r;
		x4 = a + r;
		y4 = d - r;
		break;
	case RADIUS:
		x1 = a - c + r;
		y1 = b - d + r;
		x2 = a + c - r;
		y2 = b - d + r;
		x3 = a + c - r;
		y3 = b + d - r;
		x4 = a - c + r;
		y4 = b + d - r;
		break;
	case CENTER:
		x1 = a - (c / 2) + r;
		y1 = b - (d / 2) + r;
		x2 = a + (c / 2) - r;
		y2 = b - (d / 2) + r;
		x3 = a + (c / 2) - r;
		y3 = b + (d / 2) - r;
		x4 = a - (c / 2) + r;
		y4 = b + (d / 2) - r;
		break;
	}

	//draw the inner portion and sides
	quad(x1 - r, y1, x2 + r, y2, x3 + r, y3, x4 - r, y4);
	//draw the top side
	quad(x1, y1-r, x2, y2-r, x3, y2, x4, y1);
	//draw the bottom side
	quad(x1, y4, x2, y3, x3, y3+r, x4, y4+r);
	int CORNER_RES = 10;
	//draw upper left corner
	for (int i = 0; i < CORNER_RES; ++i)
	{
		float cosVal0 = cos((i       / (float)CORNER_RES) * HALF_PI) * r;
		float sinVal0 = sin((i       / (float)CORNER_RES) * HALF_PI) * r;
		float cosVal1 = cos(((i + 1) / (float)CORNER_RES) * HALF_PI) * r;
		float sinVal1 = sin(((i + 1) / (float)CORNER_RES) * HALF_PI) * r;
		//draw upper left corner
		triangle( x1, y1, x1 - cosVal0, y1 - sinVal0, x1 - cosVal1, y1 - sinVal1);
		//draw upper right corner
		triangle(x2, y2, x2 + cosVal0, y2 - sinVal0, x2 + cosVal1, y2 - sinVal1);
		//draw lower right corner
		triangle(x3, y3, x3 + cosVal0, y3 + sinVal0, x3 + cosVal1, y3 + sinVal1);
		//draw lower left corner
		triangle(x4, y4, x4 - cosVal0, y4 + sinVal0, x4 - cosVal1, y4 + sinVal1);
	}
}

void PApplet::rectMode(RectMode mode)
{
	gState.rectMode = mode;
}

void PApplet::background(Color color)
{
	FLOAT clearColor[4] = { color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a/255.0f};
	gState.d3dImmediateContext->ClearRenderTargetView(gState.renderTargetView, clearColor);
}

float PApplet::cos(float angle)
{
	return cosf(angle);
}

float PApplet::sin(float angle)
{
	return sinf(angle);
}

float PApplet::random(float high)
{
	int r = rand();
	float x =  r / (float)RAND_MAX;
	float result = x * high;
	return result;
}

float PApplet::random(float low, float high)
{
	int r = rand();
	float x = r / (float)RAND_MAX;
	float result = x * (high - low);
	return low + result;
}

float PApplet::mouseX()
{
	return gState.mouseX;
}
float PApplet::mouseY()
{
	return gState.mouseY;
}

int PApplet::mint(int a, int b)
{
	return a < b ? a : b;
}

int PApplet::maxt(int a, int b)
{
	return a > b ? a : b;
}
