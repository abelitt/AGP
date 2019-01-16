#pragma once
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
//#include <d3d11.h>
#include <dinput.h>
//#include <bitset>
class Input
{
private:
	//unsigned char g_keyboard_keys_state[256];
	IDirectInput8* g_direct_input;
	IDirectInputDevice8* g_keyboard_device;
	unsigned char g_keyboard_keys_state[256];
public:
	//IDirectInput8* g_direct_input;
	//IDirectInputDevice8* g_keyboard_device;
	Input();
	HRESULT InitialiseInput(HINSTANCE instance, HWND hWnd);
	void ReadInputStates();
	bool IsKeyPressed(unsigned char DI_keycode);
	~Input();
};

