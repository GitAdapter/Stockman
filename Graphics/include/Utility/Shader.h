#pragma once
#include <d3d11.h>
#include <initializer_list>
#include <Resources\Resources.h>
#include "../../export.h"

namespace Graphics
{
	enum GRAPHICS_API ShaderType
	{
		VS = 1 << 0,
		PS = 1 << 1,
		GS = 1 << 2
	};

	ShaderType GRAPHICS_API operator|(ShaderType a, ShaderType b);

	class GRAPHICS_API Shader
	{
	public:
		typedef size_t Flags;

        Shader(Resources::Shaders::Files shader, ShaderType shaderType = VS | PS, std::initializer_list<D3D11_INPUT_ELEMENT_DESC> input_desc = {});
        Shader(ID3D11Device * device, Resources::Shaders::Files shader, std::initializer_list<D3D11_INPUT_ELEMENT_DESC> inputDesc = {}, ShaderType shaderType = VS | PS);
        Shader(ID3D11Device * device, LPCWSTR shaderPath, std::initializer_list<D3D11_INPUT_ELEMENT_DESC> inputDesc = {}, ShaderType shaderType = VS | PS);
		virtual ~Shader();

		inline operator ID3D11InputLayout*() const { return inputLayout ? inputLayout : throw "Shader has no Input Layout"; }
		inline operator ID3D11VertexShader*() const { return vertexShader ? vertexShader : throw "Shader has no Vertex Shader"; }
		inline operator ID3D11GeometryShader*() const { return geometryShader ? geometryShader : throw "Shader has no Geometry Shader"; }
		inline operator ID3D11PixelShader*()  const { return pixelShader ? pixelShader : throw "Shader has no Pixel Shader"; }

		void recompile(ID3D11Device * device, LPCWSTR shaderPath, std::initializer_list<D3D11_INPUT_ELEMENT_DESC> inputDesc = {}, ShaderType shaderType = VS | PS);
	private:
		ID3D11InputLayout  * inputLayout;
		ID3D11VertexShader * vertexShader;
		ID3D11GeometryShader * geometryShader;
		ID3D11PixelShader  * pixelShader;
	};

	class ComputeShader
	{
	public:
        ComputeShader(Resources::Shaders::Files shader);
        ComputeShader(ID3D11Device * device, LPCWSTR shaderPath);
		virtual ~ComputeShader();
		void recompile(ID3D11Device * device, LPCWSTR shaderPath);

		//void setShader(ID3D11DeviceContext * deviceContext);
		inline operator ID3D11ComputeShader*() const { return computeShader; };
	private:
		ID3D11ComputeShader * computeShader;
	};
}