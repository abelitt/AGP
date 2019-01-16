#include "SkyBox.h"

HRESULT SkyBox::LoadSkyBox(ID3D11Device* device)
{
	HRESULT hr;
	D3D11_RASTERIZER_DESC rasterizer_desc;
	ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));

	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	hr = device->CreateRasterizerState(&rasterizer_desc, &g_pRasterSolid);

	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_FRONT;
	hr = device->CreateRasterizerState(&rasterizer_desc, &g_pRasterSkyBox);


	D3D11_DEPTH_STENCIL_DESC DSDecsc;
	ZeroMemory(&DSDecsc, sizeof(DSDecsc));
	DSDecsc.DepthEnable = true;
	DSDecsc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDecsc.DepthFunc = D3D11_COMPARISON_LESS;
	hr = device->CreateDepthStencilState(&DSDecsc, &g_pDepthWriteSolid);
	DSDecsc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = device->CreateDepthStencilState(&DSDecsc, &g_pDepthWriteSkyBox);



	return E_NOTIMPL;
}



SkyBox::SkyBox(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	LoadSkyBox(device);
	skyboxModel = new Model(device, deviceContext);
}



SkyBox::~SkyBox()
{
	delete skyboxModel;
}
