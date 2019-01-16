#include "Camera.h"



Camera::Camera(float x, float y, float z, float cameraRotation, float cameraPitch)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_camera_rotation = cameraRotation;
	m_camera_pitch = cameraPitch;

	m_dx = sin(m_camera_rotation * (XM_PI / 180.0)); //MAYBE CONVert to RADIANS 
	m_dz = cos(m_camera_rotation * (XM_PI / 180.0));
	m_dy = tan(m_camera_rotation * (XM_PI / 180.0));
	m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
	m_lookat = XMVectorSet(m_x + m_dx, m_y + m_dy, m_z + m_dz, 0.0);
	m_up = XMVectorSet(0.0, 1.0, 0.0, 0.0);
	// view = XMMatrixLookAtLH(position, lookat, up);
}

void Camera::Rotate(float radians)
{
	m_camera_rotation += XMConvertToRadians(radians);
	m_dx = sin(m_camera_rotation);
	m_dz = cos(m_camera_rotation);
	//m_position += XMVectorSet(m_x, m_y, m_z, 0.0);
}

void Camera::Pitch(float radians)
{
	if (fabs(m_camera_pitch + XMConvertToRadians(radians)) < XMConvertToRadians(90)) //If adding on the pitch (radians) increase the pitch (m_camera_pitch) beyond 90 degrees, do not allow this to run. other wise move the head
	{
		m_camera_pitch += XMConvertToRadians(radians);
		m_dy = tan(m_camera_pitch);
	}
}

void Camera::Forward(float distance)
{
	m_x += m_dx*distance;
	m_z += m_dz*distance;
}
void Camera::Strafe(float distance)
{
	/*
	i can't do it
	i can't do it
	i can't do it
	i can't do it
	i can't do it
	i can't do it
	i can't do it
	i can't do it
	i can't do it
	i can't do it
	i can't do it*/
	XMFLOAT4 t;
	XMStoreFloat4(&t, m_up);
	XMFLOAT4 forward = XMFLOAT4(m_dx, 0, m_dz, 0);
	float directionX = (forward.y*t.z) - (forward.z*t.y);
	float directionZ = (forward.x*t.y) - (forward.y*t.x);
	m_x += directionX * distance;
	m_z += directionZ * distance;
}
void Camera::Jump(float distance)
{
	if (m_y <= jumpLimit)
	{
		jumpLimit = 3; //MaxJumpLimit
		m_y += distance;
	}
	else
	{
		jumpLimit = 0;
	}
}
XMMATRIX Camera::GetViewMatrix()
{
	if (m_y > 0)
	{
		if (m_y - gravity < 0)
		{
			m_y = 0;
		}
		else
		{
			m_y -= gravity;
		}
	}

	m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
	m_lookat = XMVectorSet(m_x + m_dx, m_y + m_dy, m_z + m_dz, 0.0);
	m_up = XMVectorSet(0.0, 1.0, 0.0, 0.0);
	return XMMatrixLookAtLH(m_position, m_lookat, m_up);
}
float Camera::GetX()
{
	return m_x;
}
float Camera::GetY()
{
	return m_y;
}
float Camera::GetZ()
{
	return m_z;
}

float Camera::GetDX()
{
	return m_dx;
}

float Camera::GetDY()
{
	return m_dy;
}

float Camera::GetDZ()
{
	return m_dz;
}

void Camera::SetX(float input)
{
	m_x = input;
}

void Camera::SetY(float input)
{
	m_y = input;
}

void Camera::SetZ(float input)
{
	m_z = input;
}


//CONSIDER ADDING UP
Camera::~Camera()
{
}
