#pragma once
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <d3d11.h>
#include <math.h>
#include <xnamath.h>

class Camera
{
private:
	float m_x, m_y, m_z, m_dx, m_dy, m_dz, m_camera_rotation, m_camera_pitch;
	XMVECTOR m_position, m_lookat, m_up;
	bool jumping = false;
	float jumpLimit = 1.0f;
	float gravity = 0.001f;

public:
	Camera(float x, float y, float z, float cameraRotation, float cameraPitch);
	void Rotate(float radians);
	void Pitch(float radians);
	void Forward(float distance);
	void Strafe(float distance);
	void Jump(float distance);
	XMMATRIX GetViewMatrix();
	float GetX();
	float GetY();
	float GetZ();
	float GetDX();
	float GetDY();
	float GetDZ();
	
	void SetX(float input);
	void SetY(float input);
	void SetZ(float input);
	~Camera();
	
};

