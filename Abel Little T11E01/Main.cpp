//The #include order is important
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <windows.h>
#include <dxerr.h>
#include <xnamath.h>
#include "Camera.h"
#include "text2D.h"
#include "Model.h"
#include "Input.h"
#include "SkyBox.h"
#include <iostream>
#include <sstream>


//////////////////////////////////////////////////////////////////////////////////////
//	Global Variables
//////////////////////////////////////////////////////////////////////////////////////
HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;
float		WINDOW_SIZE_X = 640;
float		WINDOW_SIZE_Y = 480;

float directional_light_x = 0.0f;
float directional_light_y = 0.0f;
float directional_light_z = 0.0f;

// Rename for each tutorial � This will appear in the title bar of the window
char		g_TutorialName[100] = "Tutorial 08 Exercise 02\0";

D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pD3DDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;

ID3D11RenderTargetView* g_pBackBufferRTView = NULL;

ID3D11Buffer*		g_pVertexBuffer;
ID3D11VertexShader*	g_pVertexShader;
ID3D11PixelShader*	g_pPixelShader;
ID3D11InputLayout*	g_pInputLayout;
ID3D11DepthStencilView* g_pZBuffer;
ID3D11ShaderResourceView* g_pTexture0; //T08
ID3D11SamplerState* g_pSampler0; //T08

//T09
XMVECTOR g_directional_light_shines_from;
XMVECTOR g_directional_light_colour;
XMVECTOR g_ambient_light_colour;

POINT previousPointerPos;
float mouseX;
float mouseY;

ID3D11Buffer* g_pConstantBuffer0;
Camera* g_pCam;
Text2D* g_p2DText;

vector<Model*> allModels;
Model* g_baseModel;
Model* g_pSquishy;
Model* g_pDoor;
Model* g_pBox;
Model* g_pButton;
SkyBox* g_pSkyBox;
Input* g_pInput;

bool buttonPressed = false;
bool squishySquished = false;

float previous_pos_x;
float previous_pos_y;
float previous_pos_z;


//Define vertex structure
struct POS_COL_TEX_NORM_VERTEX//This will be added to and renamed in future tutorials
{
	XMFLOAT3	pos;
	XMFLOAT4	Col;
	XMFLOAT2	Texture0; //T08
	XMFLOAT3    Normal;
};

struct CONSTANT_BUFFER0
{
	XMMATRIX WorldViewProjection; // 64 bytes
	XMVECTOR directional_light_vector; //16 bytes
	XMVECTOR directional_light_colour; //16 bytes
	XMVECTOR ambient_light_colour; //16 bytes
	
}; //112 Bytes


//////////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT InitialiseGraphics(void);

HRESULT InitialiseD3D();
void ShutdownD3D();
void RenderFrame(void);
void TestInput();
void TestCollision();
float Distance(Model* obj1, Model* obj2);

//////////////////////////////////////////////////////////////////////////////////////
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_pInput = new Input();
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}

	if (FAILED(g_pInput->InitialiseInput(hInstance, g_hWnd)))
	{
		DXTRACE_MSG("Failed to create Input");
		return 0;
	}

	if (FAILED(InitialiseD3D()))
	{
		DXTRACE_MSG("Failed to create Device");
		return 0;
	}

	if (FAILED(InitialiseGraphics())) // 03-01
	{
		DXTRACE_MSG("Failed to initialise graphics");
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// do something
			RenderFrame();
		}
	}
	ShutdownD3D();
	return (int)msg.wParam;
}


//////////////////////////////////////////////////////////////////////////////////////
// Register class and create window
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Give your app your own name
	char Name[100] = "Hello World\0";

	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	//   wcex.hbrBackground = (HBRUSH )( COLOR_WINDOW + 1); // Needed for non-D3D apps
	wcex.lpszClassName = Name;

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, WINDOW_SIZE_X, WINDOW_SIZE_Y }; //640, 480
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(Name, g_TutorialName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
		rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Called every time the application receives a message
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOUSEMOVE:
		POINT currentPointerPos;
		GetCursorPos(&currentPointerPos);
		mouseX = currentPointerPos.x - previousPointerPos.x; //Difference between previous position and current position for x
		g_pCam->Rotate(mouseX/10); //Rotate cam for x
		mouseY = currentPointerPos.y - previousPointerPos.y; //Difference between previous position and current position for y
		g_pCam->Pitch(-mouseY / 5); //Rotate cam for y
		RECT foo;
		GetWindowRect(g_hWnd, &foo); //Get window dimentions
		mouseX = foo.left + (WINDOW_SIZE_X / 2); //get middle of window
		mouseY = foo.top + (WINDOW_SIZE_Y / 2);
		SetCursorPos(mouseX, mouseY); //place window middle 
		//previousPointerPos.x = 300;
		//previousPointerPos.y = 300;
		break;

	//case WM_KEYDOWN:
	//	if (wParam == VK_ESCAPE)
	//		DestroyWindow(g_hWnd);
	//	if (wParam == VK_NUMPAD7)
	//		g_pCam->Rotate(-5);
	//	if (wParam == VK_NUMPAD9)
	//		g_pCam->Rotate(5);
	//	if (wParam == VK_NUMPAD8)
	//		g_pCam->Forward(0.1);
	//	if (wParam == VK_NUMPAD5)
	//		g_pCam->Forward(-0.1);
	//	if (wParam == VK_NUMPAD4)
	//		g_pCam->Strafe(0.1f);
	//	if (wParam == VK_NUMPAD6)
	//		g_pCam->Strafe(-0.1f);
	//	if (wParam == VK_SPACE)
	//		g_pCam->Jump(1.0f);
	//	if (wParam == VK_NUMPAD1)
	//		g_pCam->Pitch(5.0f);
	//	if (wParam == VK_NUMPAD3)
	//		g_pCam->Pitch(-5.0f);
	//	if (wParam == VK_DELETE)
	//		g_pBox->IncPos(0, 0, -1);
	//	if (wParam == VK_END)
	//		g_pBox->IncPos(0, 0, 1);
	//	if (wParam == VK_UP)
	//		g_pBox->IncPos(0, -1, 0);
	//	if (wParam == VK_DOWN)
	//		g_pBox->IncPos(0, 1, 0);
	//	if (wParam == VK_LEFT)
	//		g_pBox->IncPos(-1, 0, 0);
	//	if (wParam == VK_RIGHT)
	//		g_pBox->IncPos(1, 0, 0);
	//	if (wParam == VK_HOME)
	//	{
	//		g_pBox->LookAt_XZ(g_pCam->GetX(), g_pCam->GetZ());
	//		//g_pBox->LookAt_XZ(90, 10);
	//	}
	//	if (wParam == VK_INSERT)
	//	{
	//		g_pBox->MoveForward(0.5f);
	//	}

	//	
		
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Create D3D device and swap chain
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, // comment out this line if you need to test D3D 11.0 functionality on hardware that doesn'converter support it
		D3D_DRIVER_TYPE_WARP, // comment this out also to use reference device
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain,
			&g_pD3DDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	// Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBufferTexture);

	if (FAILED(hr)) return hr;

	// Use the back buffer texture pointer to create the render target view
	hr = g_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL,
		&g_pBackBufferRTView);
	pBackBufferTexture->Release();

	if (FAILED(hr)) return hr;

	//Create a Z buffer texture
	D3D11_TEXTURE2D_DESC tex2dDesc;
	ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));

	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.ArraySize = 1;
	tex2dDesc.MipLevels = 1;
	tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex2dDesc.SampleDesc.Count = sd.SampleDesc.Count;
	tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* pZBufferTexture;
	hr = g_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);

	if (FAILED(hr)) return hr;

	// Create the Z buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &g_pZBuffer);
	pZBufferTexture->Release();


	// Set the render target view
	g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	g_pImmediateContext->RSSetViewports(1, &viewport);

	g_p2DText = new Text2D("assets/font1.bmp", g_pD3DDevice, g_pImmediateContext);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{
	if (g_pVertexBuffer)		g_pVertexBuffer->Release(); //03-01
	if (g_pInputLayout)			g_pInputLayout->Release(); //03-01
	if (g_pVertexShader)		g_pVertexShader->Release(); //03-01
	if (g_pPixelShader)			g_pPixelShader->Release(); // 03-01
	if (g_pConstantBuffer0)		g_pConstantBuffer0->Release(); //04-01
	if (g_pSwapChain)			g_pSwapChain->Release();
	if (g_pTexture0)			g_pTexture0->Release(); //08
	if (g_pSampler0)			g_pSampler0->Release(); //08
	if (g_pImmediateContext)	g_pImmediateContext->Release();
	if (g_pBackBufferRTView)	g_pBackBufferRTView->Release();
	if (g_pD3DDevice)			g_pD3DDevice->Release();
	if (g_pZBuffer)				g_pZBuffer->Release();
	if (g_pCam)
	{
		g_pCam->~Camera();
		g_pCam = nullptr;
	}
	if (g_p2DText)
	{
		g_p2DText->~Text2D();
		g_p2DText = nullptr;
	}
	if (g_pBox)
	{
		delete g_pBox;
		g_pBox = nullptr;
	}


}

/////////////////////////////////////////////////////////////////////////////////////////////
//Init graphics - Tutorial 03
///////////////////////////////////////////////////////////////////////////////////////////// 
HRESULT InitialiseGraphics()
{
	
	HRESULT hr = S_OK;

	
	//Define vertices of a triangle - screen coordinates -1.0 to +1.0
	/*
	POS_COL_VERTEX vertices[] =
	{
		{ XMFLOAT3(0.9f, 0.9f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.9f, -0.9f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.9f, -0.9f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
	};
	*/
	float posVal = 1.0f;
	POS_COL_TEX_NORM_VERTEX vertices[] =
	{
	
		//back face 
		{ XMFLOAT3(-posVal, posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(posVal, posVal, posVal),		XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(posVal, posVal, posVal),		XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

		//front face
		{ XMFLOAT3(-posVal, -posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-posVal, -posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(posVal, -posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

		//Left face
		{ XMFLOAT3(-posVal, -posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-posVal, posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		//Right face
		{ XMFLOAT3(posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(posVal, -posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(posVal, posVal, posVal),		XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		//Bottom face
		{ XMFLOAT3(posVal, -posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-posVal, -posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-posVal, -posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-posVal, -posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

		//Top Face
		{ XMFLOAT3(posVal, posVal, posVal),		XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-posVal, posVal, posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(posVal, posVal, posVal),		XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-posVal, posVal, -posVal),	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

	};
	
	//Set up and create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;										//Allows use by CPU and GPU
	bufferDesc.ByteWidth = sizeof(vertices);							//Set the total size of the buffer (in this case, 3 vertices)
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;							//Set the type of buffer to vertex buffer
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;							//Allow access by the CPU
	hr = g_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &g_pVertexBuffer);		//Create the buffer

	if (FAILED(hr))//Return an error code if failed
	{
		return hr;
	}

	
	//Create Constant buffer //T04
	
	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));

	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	constant_buffer_desc.ByteWidth = 112; //MUST BE multiple of 16
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = g_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &g_pConstantBuffer0);

	if (FAILED(hr)) return hr;
	
	//T04

	//Copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;

	//Lock the buffer to allow writing
	g_pImmediateContext->Map(g_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

	//Copy the data
	memcpy(ms.pData, vertices, sizeof(vertices)); //COMMENTED OUT CURRENTLY

	//Unlock the buffer
	g_pImmediateContext->Unmap(g_pVertexBuffer, NULL);

	//Load and compile the pixel and vertex shaders - use vs_5_0 to target DX11 hardware only 
	///ATTENTIONCLASS ATTENTUION
	ID3DBlob *VS, *PS, *error;
	hr = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don'converter fail if error is just a warning
		{
			return hr;
		}
	}

	hr = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don'converter fail if error is just a warning
		{
			return hr;
		}
	}

	//Create shader objects
	hr = g_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = g_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &g_pPixelShader);
	if (FAILED(hr))
	{
		return hr;
	}

	//Set the shader objects as active
	g_pImmediateContext->VSSetShader(g_pVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, 0, 0);

	//Create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		//Be very careful setting the correct dxgi format and D3D version
		{	"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{	"COLOR", 0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{	"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{	"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = g_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &g_pInputLayout);
	if (FAILED(hr))
	{
		return hr;
	}

	g_pImmediateContext->IASetInputLayout(g_pInputLayout);

	//T08
	

	g_pSkyBox = new SkyBox(g_pD3DDevice, g_pImmediateContext);
	g_pSkyBox->skyboxModel->AddTexture((char*)"assets/skybox01.dds");
	g_pSkyBox->skyboxModel->LoadModel((char*)"assets/Cube.obj", "sky_shaders.hlsl");
	g_pSkyBox->skyboxModel->SetScale(3, 3, 3);
	g_pSkyBox->skyboxModel->SetName("Sky Box");

	g_pBox = new Model(g_pD3DDevice, g_pImmediateContext);
	g_pBox->AddTexture((char*)"assets/heart_cube.bmp");
	g_pBox->LoadModel((char*)"assets/Cube.obj", "model_shaders.hlsl");
	g_pBox->SetPos(5, 0, 15);
	g_pBox->SetName("Box");
	allModels.push_back(g_pBox);

	g_pDoor = new Model(g_pD3DDevice, g_pImmediateContext);
	g_pDoor->AddTexture((char*)"assets/heart_cube.bmp");
	g_pDoor->LoadModel((char*)"assets/Cube.obj", "model_shaders.hlsl");
	g_pDoor->SetPos(10, 1, 5);
	g_pDoor->SetScale(0.1, 2, 1);
	g_pDoor->SetName("Door");
	allModels.push_back(g_pDoor);

	g_pButton = new Model(g_pD3DDevice, g_pImmediateContext);
	g_pButton->AddTexture((char*)"assets/heart_cube.bmp");
	g_pButton->LoadModel((char*)"assets/Cube.obj", "model_shaders.hlsl");
	g_pButton->SetScale(1, 0.1, 1);
	g_pButton->SetPos(3, -0.9, 3);
	g_pButton->SetName("Button");
	allModels.push_back(g_pButton);

	g_pSquishy = new Model(g_pD3DDevice, g_pImmediateContext);
	g_pSquishy->AddTexture((char*)"assets/bluegoo.png");
	g_pSquishy->LoadModel((char*)"assets/Cube.obj", "model_shaders.hlsl");
	g_pSquishy->SetPos(-5, 0, -5);
	g_pSquishy->SetScale(0.5, 0.5, 0.5);
	g_pSquishy->SetName("Squishy");
	allModels.push_back(g_pSquishy);

	g_baseModel = new Model(g_pD3DDevice, g_pImmediateContext);
	g_baseModel->AddTexture((char*)"assets/wall.png");
	g_baseModel->LoadModel((char*)"assets/Cube.obj", "model_shaders.hlsl");
	g_baseModel->SetPos(11, 1, 6);
	g_baseModel->SetRotationY(90);
	g_baseModel->SetScale(0.1, 2, 1);
	allModels.push_back(g_baseModel);

	g_baseModel = new Model(g_pD3DDevice, g_pImmediateContext);
	g_baseModel->AddTexture((char*)"assets/wall.png");
	g_baseModel->LoadModel((char*)"assets/Cube.obj", "model_shaders.hlsl");
	g_baseModel->SetPos(11, 1, 4);
	g_baseModel->SetRotationY(90);
	g_baseModel->SetScale(0.1, 2, 1);
	allModels.push_back(g_baseModel);

	g_baseModel = new Model(g_pD3DDevice, g_pImmediateContext);
	g_baseModel->AddTexture((char*)"assets/wall.png");
	g_baseModel->LoadModel((char*)"assets/Cube.obj", "model_shaders.hlsl");
	g_baseModel->SetPos(12, 1, 5);
	g_baseModel->SetScale(0.1, 2, 1);
	g_baseModel->SetName("model1");
	allModels.push_back(g_baseModel);

	g_baseModel = new Model(g_pD3DDevice, g_pImmediateContext);
	g_baseModel->AddTexture((char*)"assets/win.png");
	g_baseModel->LoadModel((char*)"assets/Cube.obj", "model_shaders.hlsl");
	g_baseModel->SetPos(11, 1, 5);
	g_baseModel->SetScale(0.3, 0.3, 0.3);
	allModels.push_back(g_baseModel);



	
	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	g_pD3DDevice->CreateSamplerState(&sampler_desc, &g_pSampler0);
	 
	g_pCam = new Camera(0.0, 0.0, -0.5, 0, 0);

	return S_OK;
}
/////////////////////////////////////////////////////////////////////////////////////////////


// Render frame
void RenderFrame(void) //UPDATE
{
	// Clear the back buffer - choose a colour you like
	float rgba_clear_colour[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, rgba_clear_colour);
	g_pImmediateContext->ClearDepthStencilView(g_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pInput->ReadInputStates();
	
	TestCollision();
	TestInput();
	
	GetCursorPos(&previousPointerPos);

	if (g_pSkyBox->skyboxModel->CheckCollision(g_pSquishy))
	{
		g_pSquishy->SetScale(0.5, 0.01, 0.5);
		g_pSquishy->SetPosY(-1);
		squishySquished = true;
	}
	
	if (squishySquished == false)
	{
		g_pSquishy->LookAt_XZ(g_pCam->GetX(), g_pCam->GetZ());
		g_pSquishy->MoveForward(0.0005f);
	}

	
	
	//collisions
	
	if (g_pBox->CheckCollision(g_pButton))
	{
		buttonPressed = true;
		g_pDoor->SetRotationY(90);
		g_pDoor->SetPosZ(4);
		g_pDoor->SetPosX(11);
	}
	else
	{
		buttonPressed = false;
		g_pDoor->SetRotationY(0);
		g_pDoor->SetPosZ(5);
		g_pDoor->SetPosX(10);
	}
	
	// Select which primtive type of use //03-01
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX projection, world, view;
	CONSTANT_BUFFER0 cb0_values;
	
	world =			XMMatrixRotationY(XMConvertToRadians(90));
	world =			XMMatrixTranslation(0, 0, 5);
	projection =	XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), 640.0 / 480.0, 1.0, 100.0); //Make projection matrix, making the fov and the vertical fov
	view =			g_pCam->GetViewMatrix();
	
	cb0_values.WorldViewProjection = world * view * projection;

	XMFLOAT4X4 converter;
	XMStoreFloat4x4(&converter, view);
	XMMATRIX* viewptr = new XMMATRIX(XMLoadFloat4x4(&converter));
	XMStoreFloat4x4(&converter, projection);
	XMMATRIX* projectionptr = new XMMATRIX(XMLoadFloat4x4(&converter));
	
	
	
	g_pSkyBox->skyboxModel->SetPos(g_pCam->GetX(), g_pCam->GetY(), g_pCam->GetZ());
	g_pImmediateContext->RSSetState(g_pSkyBox->g_pRasterSkyBox);
	g_pImmediateContext->OMSetDepthStencilState(g_pSkyBox->g_pDepthWriteSkyBox, NULL);
	g_pSkyBox->skyboxModel->Draw(viewptr, projectionptr);
	g_pImmediateContext->RSSetState(g_pSkyBox->g_pRasterSolid);
	g_pImmediateContext->OMSetDepthStencilState(g_pSkyBox->g_pDepthWriteSolid, NULL);

	for (int i = 0; i < allModels.size(); i++)
	{
		allModels[i]->Draw(viewptr, projectionptr);
	}

	delete viewptr;
	viewptr = nullptr;
	delete projectionptr;
	projectionptr = nullptr;
	
	std::stringstream ss(stringstream::in | stringstream::out);
	ss << buttonPressed;
	g_p2DText->AddText(ss.str(), -1.0, 1.0, .1);
	g_p2DText->RenderText();
	ss.clear();

	// Display what has just been rendered
	g_pSwapChain->Present(0, 0);
}

void TestInput()
{

	if (g_pInput->IsKeyPressed(DIK_ESCAPE)) DestroyWindow(g_hWnd);
	if (g_pInput->IsKeyPressed(DIK_W)) g_pCam->Forward(0.001);
	if (g_pInput->IsKeyPressed(DIK_S)) g_pCam->Forward(-0.001);
	if (g_pInput->IsKeyPressed(DIK_A)) g_pCam->Strafe(0.001);
	if (g_pInput->IsKeyPressed(DIK_D)) g_pCam->Strafe(-0.001);
	if (g_pInput->IsKeyPressed(DIK_SPACE)) g_pCam->Jump(0.01);

	if (g_pInput->IsKeyPressed(DIK_E))
	{
		if (g_pSkyBox->skyboxModel->CheckCollision(g_pBox)) //Check the collision for the cube is successful
		{
			g_pBox->SetPos(g_pCam->GetX()+g_pCam->GetDX()*3, 0, g_pCam->GetZ()+g_pCam->GetDZ()*3); //Put cube in front of Abel
		}	
	}


	//if (g_pInput->IsKeyPressed(DIK_NUMPAD1)) g_pSkyBox->skyboxModel->IncPos(0.01,0,0);
}


void TestCollision()
{
	g_pSkyBox->skyboxModel->SetScale(1, 1, 1); //use the skybox as collision box, by reducing the size to the player
	for (int i = 0; i < allModels.size(); i++)
	{
		if (g_pSkyBox->skyboxModel->CheckCollision(allModels[i]))
		{
			g_pCam->SetX(previous_pos_x);
			g_pCam->SetY(previous_pos_y);
			g_pCam->SetZ(previous_pos_z);
			break;
		}
	}
	g_pSkyBox->skyboxModel->SetScale(3, 3, 3); //put size of skybox back to orginal size
	previous_pos_x = g_pCam->GetX();
	previous_pos_y = g_pCam->GetY();
	previous_pos_z = g_pCam->GetZ();
}


float Distance(Model* obj1, Model* obj2)
{
	return sqrt(((obj1->GetPosX() - obj2->GetPosX()) * (obj1->GetPosX() - obj2->GetPosX())) + (obj1->GetPosY() - obj2->GetPosY() * obj1->GetPosY() - obj2->GetPosY()) + (obj1->GetPosZ() - obj2->GetPosZ() * obj1->GetPosZ() - obj2->GetPosZ()));
}