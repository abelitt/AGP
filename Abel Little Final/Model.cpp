#include "Model.h"
struct MODEL_CONSTANT_BUFFER
{
	XMMATRIX WorldViewProjection; //64 bytes (4x4 = 16 floats x 4 bytes)
	XMVECTOR directional_light_vector; //16 bytes
	XMVECTOR directional_light_colour; //16 bytes
	XMVECTOR ambient_light_colour; //16 bytes
}; //112 bytes


Model::Model(ID3D11Device * device, ID3D11DeviceContext * deviceContext)
{
	m_pD3DDevice = device;
	m_pImmediateContext = deviceContext;

	m_x = 0;
	m_y = 0;
	m_z = 0;
	m_model_rotation = 0;
	m_direction = XMVectorSet(m_x, m_y, m_z, 0.0);
	m_xAngle = 0;
	m_yAngle = 0;
	m_zAngle = 0;
	m_scale = 1.0f;
}

HRESULT Model::LoadModel(char * modelName)
{
	m_pObject = new ObjFileModel(modelName, m_pD3DDevice, m_pImmediateContext);
	
	if (m_pObject->filename == "FILE NOT LOADED") return S_FALSE;

	//m_pImmediateContext->PSSetSamplers(0, 1, &g_pSampler0);
	//m_pImmediateContext->PSSetShaderResources(0, 1, &g_pTexture0);
	

	HRESULT hr = S_OK;

	ID3DBlob *VS, *PS, *error;
	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}

	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))//Don't fail if error is just a warning
		{
			return hr;
		}
	}

	//Create shader objects
	hr = m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVShader);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPShader);
	if (FAILED(hr))
	{
		return hr;
	}



	m_pImmediateContext->VSSetShader(m_pVShader, NULL, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, NULL, 0);

	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		//Be very careful setting the correct dxgi format and D3D version
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	//Create constant buffer
	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));

	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	constant_buffer_desc.ByteWidth = 112; //MUST BE multiple of 16
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = m_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &m_pConstantBuffer);

	if (FAILED(hr)) return hr;



	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);
	if (FAILED(hr))
	{
		return hr;
	}
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);


	return E_NOTIMPL;
}

void Model::Draw(XMMATRIX* view, XMMATRIX* projection)
{
	MODEL_CONSTANT_BUFFER model_cb_values;
	XMMATRIX world, transpose;


	m_directional_light_shines_from = XMVectorSet(m_directional_light_x, m_directional_light_y, m_directional_light_z, 0.0f);
	m_directional_light_colour = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);//green
	m_ambient_light_colour = XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f);//dark grey - always use a small value for ambient lighting

	transpose = XMMatrixTranspose(world);

	model_cb_values.directional_light_colour = m_directional_light_colour;
	model_cb_values.ambient_light_colour = m_ambient_light_colour;
	model_cb_values.directional_light_vector = XMVector3Transform(m_directional_light_shines_from, transpose);
	model_cb_values.directional_light_vector = XMVector3Normalize(model_cb_values.directional_light_vector);



	world = XMMatrixScaling(m_scale, m_scale, m_scale);
	world *= XMMatrixRotationZ(XMConvertToRadians(m_zAngle));
	world *= XMMatrixRotationX(XMConvertToRadians(m_xAngle));
	world *= XMMatrixRotationY(XMConvertToRadians(m_yAngle));
	world *= XMMatrixTranslation(m_x, m_y, m_z);
	model_cb_values.WorldViewProjection = world*(*view)*(*projection);
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &model_cb_values, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pImmediateContext->VSSetShader(m_pVShader, NULL, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, NULL, 0);
	m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTexture);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);
	
	

	m_pObject->Draw();
}

void Model::AddTexture(char * fileName)
{
	D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice, fileName, NULL, NULL, &m_pTexture, NULL);
}

void Model::SetPosX(float x)
{
	m_x = x;
}

void Model::SetPosY(float y)
{
	m_y = y;
}

void Model::SetPosZ(float z)
{
	m_z = z;
}

void Model::SetPos(float x, float y, float z)
{
	SetPosX(x);
	SetPosY(y);
	SetPosZ(z);
}

void Model::SetRotationX(float x)
{
	m_xAngle = x;
}

void Model::SetRotationY(float y)
{
	m_yAngle = y;
}

void Model::SetRotationZ(float z)
{
	m_zAngle = z;
}

void Model::SetRotation(float x, float y, float z)
{
	SetRotationX(x);
	SetRotationY(y);
	SetRotationZ(z);
}

void Model::SetScale(float scale)
{
	m_scale = scale;
}

float Model::GetPosX()
{
	return m_x;
}

float Model::GetPosY()
{
	return m_y;
}

float Model::GetPosZ()
{
	return m_z;
}

float Model::GetRotationX()
{
	return m_xAngle;
}

float Model::GetRotationY()
{
	return m_yAngle;
}

float Model::GetRotationZ()
{
	return m_zAngle;
}

float Model::GetScale()
{
	return m_scale;
}

void Model::LookAt_XZ(float x, float z)
{
	
	float angle = atan2(x, z) * (180.0/XM_PI);

	
	SetRotation(GetRotationX(), angle, GetRotationZ());
}

void Model::IncPos(float x, float y, float z)
{
	m_x += x;
	m_y += y;
	m_z += z;
}

void Model::IncRotation(float x, float y, float z)
{
	m_xAngle += x;
	m_yAngle += y;
	m_zAngle += z;
}

void Model::IncScale(float scale)
{
	m_scale = scale;
}

void Model::MoveForward(float distance)
{

	//m_x += sin(m_yAngle * (XM_PI / 180.0) * distance);
	//m_z += cos(m_yAngle * (XM_PI / 180.0) * distance);
	float y_angle = XMConvertToRadians(m_yAngle);
	m_x += sin(m_yAngle * (XM_PI/180.0))*distance;
	m_z += cos(m_yAngle * (XM_PI / 180.0))*distance;

}


Model::~Model()
{
	if(m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pPShader) m_pPShader->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pVShader) m_pVShader->Release();
	if (m_pObject) delete m_pObject;
	m_pObject = nullptr;
}


