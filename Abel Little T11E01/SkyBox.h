#pragma once
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include "Model.h"

class SkyBox
{
public:
	ID3D11RasterizerState*	 g_pRasterSolid = 0;
	ID3D11RasterizerState*	 g_pRasterSkyBox = 0;
	ID3D11DepthStencilState* g_pDepthWriteSolid = 0;
	ID3D11DepthStencilState* g_pDepthWriteSkyBox = 0;
	Model* skyboxModel;
	
	HRESULT LoadSkyBox(ID3D11Device* device);
	
	SkyBox(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~SkyBox();
};

