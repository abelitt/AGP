#pragma once
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <xnamath.h>
#include <string>
#include "objfilemodel.h"


class Model
{
private:
	std::string m_name;
	ID3D11Device*			m_pD3DDevice;
	ID3D11DeviceContext*	m_pImmediateContext;
	ObjFileModel*			m_pObject;
	ID3D11VertexShader*		m_pVShader;
	ID3D11PixelShader*		m_pPShader;
	ID3D11InputLayout*		m_pInputLayout;
	ID3D11Buffer*			m_pConstantBuffer;
	ID3D11ShaderResourceView* m_pTexture;
	float					m_x, m_y, m_z;
	float					m_dx, m_dy, m_dz;
	float					m_model_rotation;
	float					m_xAngle, m_yAngle, m_zAngle;
	float					m_xScale, m_yScale, m_zScale;
	float					m_directional_light_x = 0.0f;
	float					m_directional_light_y = 0.0f;
	float					m_directional_light_z = 0.0f;

	float					m_bounding_sphere_centre_x;
	float					m_bounding_sphere_centre_y;
	float					m_bounding_sphere_centre_z;
	float					m_bounding_sphere_radius;

	void					CalculateModelCentrePoint();
	void					CalculateBoundingSphereRadius();


	XMVECTOR				m_directional_light_shines_from;
	XMVECTOR				m_directional_light_colour;
	XMVECTOR				m_ambient_light_colour;

	//float				m_model_direction;
	float				m_model_pitch;
	XMVECTOR			m_direction;
	
public:
	Model(ID3D11Device* device = NULL, ID3D11DeviceContext* deviceContext = NULL);
	HRESULT LoadModel(char* modelName, char * shader);
	void Draw(XMMATRIX* view, XMMATRIX* projection);
	void AddTexture(char* fileName);
	
	float GetBoundingSphereRadius();

	bool CheckCollision(Model* other);

	XMVECTOR GetBoundingSphereWorldSpacePosition();


	void SetPosX(float x);
	void SetPosY(float y);
	void SetPosZ(float z);
	void SetPos(float x, float y, float z);
	void SetRotationX(float x);
	void SetRotationY(float y);
	void SetRotationZ(float z);
	void SetRotation(float x, float y, float z);
	
	void SetScale(float scaleX, float scaleY, float scaleZ);

	float GetPosX();
	float GetPosY();
	float GetPosZ();
	float GetRotationX();
	float GetRotationY();
	float GetRotationZ();
	float GetScaleX();
	float GetScaleY();
	float GetScaleZ();

	void SetName(string name);
	std::string GetString();

	void LookAt_XZ(float x, float z);

	void IncPos(float x, float y, float z);
	void IncRotation(float x, float y, float z);
	void IncScale(float x, float y, float z);
	
	void MoveForward(float distance);

	~Model();
};

