#include "PPP.h"

#include <windows.h>
#include <d3d11.h>

#include <assert.h>

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
	IDXGISwapChain* swapChain;
	float width     = DEFAULT_WINDOW_WIDTH;
	float height    = DEFAULT_WINDOW_HEIGHT;
	float frameRate = DEFAULT_FRAME_RATE;
};

static WinApplicationState gWinAppState;


void Update()
{
}

void Draw()
{
	draw(); //call out to game code
		
	gWinAppState.swapChain->Present(0, 0);

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


	gWinAppState.hInstance = hInstance;

	static TCHAR szWindowClass[] = TEXT("win32app");
	static TCHAR szTitle[]       = TEXT("My Game");

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
		gWinAppState.width, gWinAppState.height,
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
	gWinAppState.hWindow = hWnd;

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

		UINT m4xMsaaQuality;
		d3dDevice->CheckMultisampleQualityLevels(
			DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);
		assert(m4xMsaaQuality > 0);

		//create swap chain
		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width                   = gWinAppState.width;
		sd.BufferDesc.Height                  = gWinAppState.height;
		sd.BufferDesc.RefreshRate.Numerator   = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SampleDesc.Count                   = 1; //don't use msaa
		sd.SampleDesc.Quality                 = 0;
		sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount                        = 1;
		sd.OutputWindow                       = hWnd;
		sd.Windowed                           = true;
		sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags                              = 0;

		IDXGIDevice* dxgiDevice = 0;
		d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
		IDXGIAdapter* dxgiAdapter = 0;
		dxgiDevice->GetAdapter(&dxgiAdapter);
		IDXGIFactory* dxgiFactory = 0;
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

		IDXGISwapChain* mSwapChain;
		dxgiFactory->CreateSwapChain(d3dDevice, &sd, &mSwapChain);

		dxgiDevice->Release();
		dxgiAdapter->Release();
		dxgiFactory->Release();

		//create the render target view
		ID3D11RenderTargetView* mRenderTargetView;
		ID3D11Texture2D* backBuffer;
		mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
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
		vp.Width    = gWinAppState.width;
		vp.Height   = gWinAppState.height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 0.0f;
		d3dImmediateContext->RSSetViewports(1, &vp);

		gWinAppState.d3dDevice = d3dDevice;
		gWinAppState.d3dImmediateContext = d3dImmediateContext;
		gWinAppState.renderTargetView = mRenderTargetView;
		gWinAppState.swapChain = mSwapChain;
		gWinAppState.d3dInitialized = true;
	}

	

	__int64 freq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	double period = 1.0 / freq;

	double dt = 0;
	__int64 prevCount;
	__int64 counts;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevCount);
	counts = prevCount;
	static const float framePeriod = 1.0f / gWinAppState.frameRate;


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
			dt += (counts - prevCount) * period;
			prevCount = counts;

			if (dt > framePeriod)
			{
				Update();
				Draw();
				dt = 0;
			}
		}

	}

	return (int)msg.wParam;
}



//Color -----------------------------------------------------------------------------------
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

//string ---------------------------------------------------------------------------------------------

String::String(char* data)
{
	m_length = strlen(data);
	m_data = new char[m_length + 1];
	memcpy(m_data, data, m_length + 1);
}

String::String(const String& str, int offset, int length)
{
	if (m_data)
	{
		delete[] m_data;
	}
	m_length = length;
	m_data = new char[length + 1];
	memcpy(m_data, str.m_data + offset, length);
	m_data[m_length] = '\0';
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
	return *this;
}
String String::toUpperCase() const
{
	return *this;
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


// Processing functions -----------------------------------------------------------------------------------

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
	gWinAppState.width = width;
	gWinAppState.height = height;

	//todo recreate render target
}

void frameRate(float frameRate)
{
	gWinAppState.frameRate = frameRate;
}

void background(Color color)
{
	FLOAT clearColor[4] = { color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a/255.0f};
	gWinAppState.d3dImmediateContext->ClearRenderTargetView(gWinAppState.renderTargetView, clearColor);
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