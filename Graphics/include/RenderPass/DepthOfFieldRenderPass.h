#pragma once
#include "RenderPass.h"
#include "../Utility/PingPongBuffer.h"
#include "../Utility/Shader.h"
#include <memory>


namespace Graphics 
{
    class DepthOfFieldRenderPass : public RenderPass
    {
    public:
        DepthOfFieldRenderPass(
            PingPongBuffer * backBuffers,
            std::initializer_list<ID3D11RenderTargetView*> targets,
            std::initializer_list<ID3D11ShaderResourceView*> resources = {},
            std::initializer_list<ID3D11Buffer*> buffers = {},
            ID3D11DepthStencilView * depthStencil = nullptr);
        virtual ~DepthOfFieldRenderPass() {};

        virtual wchar_t* name() const override {
            return L"DepthOfFieldRenderPass";
        }

        void update(float deltaTime);
        void render() const;

    private:

        void updateCoc(float focalLength = 0.066f, float focalPlane = 0.089f, float apature = 0.913f);

        ShaderResource blur1col0;
        ShaderResource blur1col1;
        ShaderResource blur2Final;
        
        Shader CoCcreation;
        Shader blur1;
        Shader blur2;
        Shader glue;

        ConstantBuffer<float> cbuffer;
        bool firstTime;
        bool run;

        PingPongBuffer * backBuffers;
    };
}