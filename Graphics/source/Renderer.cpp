#include "Renderer.h"
#include <stdio.h>
#include "ThrowIfFailed.h"


using namespace Graphics;

Renderer::Renderer(ID3D11Device * device, ID3D11DeviceContext * deviceContext, ID3D11RenderTargetView * backBuffer)
    : device(device)
    , deviceContext(deviceContext)
    , backBuffer(backBuffer)
{
	TestQuad defferedTest[] =
	{
		DirectX::SimpleMath::Vector3(-1, -1, 0),
		DirectX::SimpleMath::Vector2(0, 1),
		DirectX::SimpleMath::Vector3(0, 0, -1),
		1,

		DirectX::SimpleMath::Vector3(-1, 1, 0),
		DirectX::SimpleMath::Vector2(0, 0),
		DirectX::SimpleMath::Vector3(0, 0, -1),
		1,

		DirectX::SimpleMath::Vector3(1, -1, 0),
		DirectX::SimpleMath::Vector2(1, 1),
		DirectX::SimpleMath::Vector3(0, 0, -1),
		1,

		DirectX::SimpleMath::Vector3(1, 1, 0),
		DirectX::SimpleMath::Vector2(1, 0),
		DirectX::SimpleMath::Vector3(0, 0, -1),
		1
	};

    createGBuffer();

	D3D11_INPUT_ELEMENT_DESC desc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}	
	};

	shaders[0] = shaderHandler.createVertexShader(device, L"FullscreenQuad.hlsl", "VS", desc, ARRAYSIZE(desc));
	shaders[1] = shaderHandler.createPixelhader(device, L"FullscreenQuad.hlsl", "PS");

	D3D11_INPUT_ELEMENT_DESC descDeffered[] =
	{
		{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "MATERIAL", 0, DXGI_FORMAT_R32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	defferedShaders[0] = shaderHandler.createVertexShader(device, L"Deffered.hlsl", "VS", descDeffered, ARRAYSIZE(descDeffered));
	defferedShaders[1] = shaderHandler.createPixelhader(device, L"Deffered.hlsl", "PS");

	D3D11_VIEWPORT viewPort;
	viewPort.Height = 720;
	viewPort.Width = 1280;
	viewPort.MaxDepth = 1.f;
	viewPort.MinDepth = 0.f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;

	deviceContext->RSSetViewports(1, &viewPort);

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.ByteWidth = sizeof(FSQuadVerts);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = FSQuadVerts;

	ThrowIfFailed(device->CreateBuffer(&bufferDesc, &data, &FSQuad2));
	ThrowIfFailed( DirectX::CreateWICTextureFromFile(device, L"cat.jpg", nullptr, &view));


	bufferDesc.ByteWidth = sizeof(defferedTest);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	data.pSysMem = defferedTest;

	ThrowIfFailed(device->CreateBuffer(&bufferDesc, &data, &defferedTestBuffer));
}

Graphics::Renderer::~Renderer()
{
	view->Release();
	FSQuad2->Release();

	if (instanceBuffer)
		instanceBuffer->Release();

	if (gbuffer.depth)
		gbuffer.depth->Release();
	if (gbuffer.depthView)
		gbuffer.depthView->Release();
	gbuffer.diffuseSpec->Release();
	gbuffer.diffuseSpecView->Release();
	gbuffer.normalMat->Release();
	gbuffer.normalMatView->Release();
	gbuffer.position->Release();
	gbuffer.positionView->Release();
	defferedTestBuffer->Release();
	
}

void Graphics::Renderer::createLightGrid()
{
	// TODO: create CS shader
	D3DCompileFromFile("", nullptr, nullptr, "CS", "cs_5_0", 0, 0, nullptr, nullptr);

	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		desc.ByteWidth = (sizeof(float) * 4) * 3600;

		ThrowIfFailed(device->CreateBuffer(&desc, nullptr, &gridFrustrums));
	}

	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.Flags = 0;
		desc.Buffer.NumElements = 3600;

		ThrowIfFailed(device->CreateUnorderedAccessView(gridFrustrums, &desc, &gridFrustrumsUAV));
	}

	deviceContext->CSSetShader(gridFrustumGenerationCS, nullptr, 0);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &gridFrustrumsUAV, 0);
	deviceContext->Dispatch(5, 3, 1);

	gridFrustrumsUAV->Release();

	// release generation shader
}

void Renderer::render(Camera * camera)
{
    /*
    //setCamera(sun);
    //setShader(shadow);
    //setRenderTarget(shadowMap);
    //for (RenderInfo info : renderQueue)
    //{
    //  draw(info);
    //}

    setCamera(camera);
    setShader(deffered);
    
*/

    //deviceContext->PSSetConstantBuffers(0, 3, nullptr);
    //deviceContext->OMSetRenderTargets(3, (ID3D11RenderTargetView * const *)&gbuffer, gbuffer.depth);
    

    ID3D11Buffer *cameraBuffer[] = { camera->getBuffer() };
    deviceContext->VSSetConstantBuffers(0, 1, cameraBuffer);    
    cull();
    //draw();
	
	//temp
	this->drawDeffered();
	this->drawToBackbuffer(gbuffer.positionView);
}

void Renderer::qeueuRender(RenderInfo * renderInfo)
{
    renderQueue.push_back(renderInfo);
}

void Renderer::createGBuffer()
{
    D3D11_TEXTURE2D_DESC textureDesc = { 0 };
    textureDesc.Width = 1280;
    textureDesc.Height = 720;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.ArraySize = 1;

    ID3D11Texture2D * diffuseSpecTexture;
    ID3D11Texture2D * normalMatTexture;
    ID3D11Texture2D * positionTexture;

    ThrowIfFailed(device->CreateTexture2D(&textureDesc, nullptr, &diffuseSpecTexture));
    ThrowIfFailed(device->CreateTexture2D(&textureDesc, nullptr, &normalMatTexture));
    ThrowIfFailed(device->CreateTexture2D(&textureDesc, nullptr, &positionTexture));
    ThrowIfFailed(device->CreateRenderTargetView(diffuseSpecTexture, nullptr, &gbuffer.diffuseSpec));
    ThrowIfFailed(device->CreateRenderTargetView(normalMatTexture, nullptr, &gbuffer.normalMat));
    ThrowIfFailed(device->CreateRenderTargetView(positionTexture, nullptr, &gbuffer.position));
	ThrowIfFailed(device->CreateShaderResourceView(diffuseSpecTexture, nullptr, &gbuffer.diffuseSpecView));
	ThrowIfFailed(device->CreateShaderResourceView(normalMatTexture, nullptr, &gbuffer.normalMatView));
	ThrowIfFailed(device->CreateShaderResourceView(positionTexture, nullptr, &gbuffer.positionView));

	diffuseSpecTexture->Release();
	normalMatTexture->Release();
	positionTexture->Release();
}

void Renderer::cull()
{
    for (RenderInfo * info : renderQueue)
    {
        if (info->render)
        {
            instanceQueue[info->meshId].push_back({ info->translation });
        }
    }
    renderQueue.clear();
}

void Renderer::draw()
{
    // Sort instance id from all meshes
    D3D11_MAPPED_SUBRESOURCE data = { 0 };

	//TODO: DEN D�R SAKEN BEH�VEER NOG INITIERAS F�RST (instanceBuffer)
    deviceContext->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);

    DWORD offset = 0;
    for (InstanceQueue_t::value_type & pair : instanceQueue)
    {
        memcpy((void*)((DWORD)data.pData + offset), &pair.second.begin(), sizeof(pair.second));
        offset += sizeof(pair.second);
    }
    
    deviceContext->Unmap(instanceBuffer, 0);


    // draw all instanced meshes
    for (InstanceQueue_t::value_type & pair : instanceQueue)
    {
        //getVertexBuffer(pair.first);
        //deviceContext->DrawInstanced()
    }
}

void Renderer::drawDeffered()
{
	
	ID3D11RenderTargetView * RTVS[] = 
	{
		gbuffer.diffuseSpec,
		gbuffer.normalMat,
		gbuffer.position
	};

	deviceContext->OMSetRenderTargets(3, RTVS, nullptr);
	shaderHandler.setShaders(defferedShaders[0], NO_SHADER, defferedShaders[1], deviceContext);

	//textur h�r typ

	UINT stride = 36, offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &defferedTestBuffer, &stride, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	deviceContext->Draw(4, 0);

	ID3D11RenderTargetView * RTVNULLS[] =
	{
		NULL,
		NULL,
		NULL
	};
	deviceContext->OMSetRenderTargets(3, RTVNULLS, NULL);
}

void Graphics::Renderer::drawToBackbuffer(ID3D11ShaderResourceView * texture)
{
    deviceContext->PSSetShaderResources(0, 1, &texture);
    UINT stride = sizeof(DirectX::SimpleMath::Vector2), offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &FSQuad2, &stride, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	deviceContext->OMSetRenderTargets(1, &backBuffer, nullptr);
    
	shaderHandler.setShaders(shaders[0], NO_SHADER, shaders[1], deviceContext);

	deviceContext->Draw(4, 0);


	ID3D11ShaderResourceView * SRVNULL = nullptr;
	deviceContext->PSSetShaderResources(0, 1, &SRVNULL);
}

