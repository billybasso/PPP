#include "PPP.h"

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

#include <stdlib.h>
#include <assert.h>

#pragma comment(lib, "d3d11")

static const int DEFAULT_WINDOW_WIDTH = 640;
static const int DEFAULT_WINDOW_HEIGHT = 360;
static const float DEFAULT_FRAME_RATE = 60;

struct WinApplicationState
{
	HINSTANCE hInstance;
	HWND hWindow;
	bool d3dInitialized = false;
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
};

static WinApplicationState gState;


void Update()
{
}

void Draw()
{
	draw(); //call out to game code
	
	gState.d3dImmediateContext->RSSetState(gState.rasterizerState);
	gState.d3dImmediateContext->DrawIndexed(6, 0, 0);
	gState.swapChain->Present(0, 0);

	gState.d3dImmediateContext->RSSetState(0); //restore the default state.

}

void OnMouseDown(WPARAM p, int x, int y)
{
	OutputDebugString(TEXT("Mouse Down!\n"));
}

void OnMouseUp(WPARAM p, int x, int y)
{
	OutputDebugString(TEXT("Mouse Up!\n"));
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
		OnMouseDown(wParam, ((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, ((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
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

	setup(); //call out to game code

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

	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		gState.width, gState.height,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hWnd)
	{
		MessageBox(NULL,
			TEXT("Call to Create Window failed!"),
			TEXT("Win32 Guided Tour"),
			NULL);

		return 1;
	}
	gState.hWindow = hWnd;

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

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
		sd.BufferDesc.Width = gState.width;
		sd.BufferDesc.Height = gState.height;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SampleDesc.Count = 1; //don't use msaa
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.OutputWindow = hWnd;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

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
		vp.Width = (FLOAT)gState.width;
		vp.Height = (FLOAT)gState.height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 0.0f;
		d3dImmediateContext->RSSetViewports(1, &vp);

		gState.d3dDevice = d3dDevice;
		gState.d3dImmediateContext = d3dImmediateContext;
		gState.renderTargetView = mRenderTargetView;
		gState.swapChain = swapChain;
		gState.d3dInitialized = true;


		struct Vertex_2DPosColor
		{
			XMFLOAT2 pos;
			XMFLOAT4 color;
		};

		

		//ID3D11XEffect* FX;
		//ID3D11EffectTechnique* tech;

		TCHAR workingDirBuff[4096];
		GetCurrentDirectory(4096, workingDirBuff);

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
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

		//create vert buffer
		XMFLOAT4 yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
		XMFLOAT4 red    = { 1.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT4 blue   = { 0.0f, 0.0f, 1.0f, 1.0f };
		XMFLOAT4 green  = { 0.0f, 1.0f, 0.0f, 1.0f };

		Vertex_2DPosColor verts[] =
		{
			{ XMFLOAT2(-1, -1), yellow },
			{ XMFLOAT2(+1, -1), red },
			{ XMFLOAT2(+1, +1), blue },
			{ XMFLOAT2(-1, +1), green }
		};

		D3D11_BUFFER_DESC vertBufferDesc;
		vertBufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
		vertBufferDesc.ByteWidth           = sizeof(Vertex_2DPosColor) * 4;
		vertBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
		vertBufferDesc.CPUAccessFlags      = 0;
		vertBufferDesc.MiscFlags           = 0;
		vertBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertInitData;
		vertInitData.pSysMem = verts;

		ID3D11Buffer* vertBuffer;
		gState.d3dDevice->CreateBuffer(&vertBufferDesc, &vertInitData, &vertBuffer);

		UINT stride = sizeof(Vertex_2DPosColor);
		UINT offset = 0;
		gState.d3dImmediateContext->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);


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


		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_NONE; //todo -- this could maybe be changed
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthClipEnable = false; //the book set this to true
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
				Update();
				Draw();
				dt = 0;
			}
			prevCount = counts;
		}
	}

	return (int)msg.wParam;
}



//Color --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Color::Color()
	: r(0), b(0), g(0), a(255) {}

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

void print(const char* text)
{
	OutputDebugStringA(text);
}

void println(const char* text)
{
	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

void println(const String& text)
{
	println(*text);
}

void size(int width, int height)
{
	gState.width = width;
	gState.height = height;

	//todo recreate render target
}

void frameRate(float frameRate)
{
	gState.frameRate = frameRate;
}

int day()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wDay;
}

int hour()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wHour;
}

int millis()
{
	return  (int)(1000.0f * (gState.totalClockCount - gState.programStartCount )/ (float)gState.clockFrequency);
}

int month()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wMonth;
}

int second()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wSecond;
}

int year()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	return systemTime.wYear;
}

void triangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	//println("drawing triangle...");
	//todo actually draw triangle
}

void background(Color color)
{
	FLOAT clearColor[4] = { color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a/255.0f};
	gState.d3dImmediateContext->ClearRenderTargetView(gState.renderTargetView, clearColor);
}

float random(float high)
{
	int r = rand();
	float x =  r / (float)RAND_MAX;
	float result = x * high;
	return result;
}

float random(float low, float high)
{
	int r = rand();
	float x = r / (float)RAND_MAX;
	float result = x * (high - low);
	return low + result;
}