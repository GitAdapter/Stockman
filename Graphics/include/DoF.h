#pragma once
#include "Utility\ShaderResource.h"
#include "Resources\Shader.h"
#include "Camera.h"
#include <CommonStates.h>
#include "Utility\DepthStencil.h"

namespace Graphics
{
    class DoF
    {
    public:
        DoF(ID3D11Device * device);
        ~DoF();
        void DoFRender(ID3D11DeviceContext * context, ShaderResource * colorBuffer, DepthStencil * depthBuffer, ShaderResource * outputBuffer, Camera *cam);

    private:

        DirectX::CommonStates *states;
        ID3D11Buffer * vertexBuffer;
        //ID3D11SamplerState * samplers[2];

        void createFullScreenQuad(ID3D11Device * device);
    };
}

