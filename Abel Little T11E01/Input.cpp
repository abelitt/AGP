#include "Input.h"



Input::Input()
{
}

HRESULT Input::InitialiseInput(HINSTANCE instance, HWND hWnd)
{
	HRESULT hr;
	ZeroMemory(g_keyboard_keys_state, sizeof(g_keyboard_keys_state));

	hr = DirectInput8Create(instance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_direct_input, NULL);
	if (FAILED(hr)) return hr;

	hr = g_direct_input->CreateDevice(GUID_SysKeyboard, &g_keyboard_device, NULL);
	if (FAILED(hr)) return hr;
	
	hr = g_keyboard_device->SetDataFormat(&c_dfDIKeyboard);

	hr = g_keyboard_device->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) return hr;

	hr = g_keyboard_device->Acquire();
	if (FAILED(hr)) return hr;

	return S_OK;
}

void Input::ReadInputStates()
{
	HRESULT hr;
	hr = g_keyboard_device->GetDeviceState(sizeof(g_keyboard_keys_state), (LPVOID)&g_keyboard_keys_state);

	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST))
		{
			g_keyboard_device->Acquire();
		}
	}
}

bool Input::IsKeyPressed(unsigned char DI_keycode)
{
	return g_keyboard_keys_state[DI_keycode] & 0x80;
}


Input::~Input()
{
	if (g_keyboard_device)
	{
		g_keyboard_device->Unacquire();
		g_keyboard_device->Release();
	}
	
	if (g_direct_input) g_direct_input->Release();


}




