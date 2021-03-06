#include "SnowManager.h"
#include <Logic\include\Misc\RandomGenerator.h>
#include <math.h>
#include <CommonStates.h>
#include <Engine\Constants.h>
#define SNOW_RADIUS 50.f
#define MAX_SNOW 512
#define PI 3.14159265f
#define ONE_DEG_IN_RAD 0.01745f

//temp
#include <Keyboard.h>

using namespace DirectX::SimpleMath;

float getRandomFloat(float from, float to)
{
    // f is [0, 1]
    float f = static_cast<float> (rand()) / static_cast<float> (RAND_MAX);
    // return [from, to]
    return (f * std::abs(to - from)) + from;
}

namespace Graphics
{

	SnowManager::SnowManager(ID3D11Device * device) :
		snowBuffer(device, CpuAccess::Write, MAX_SNOW),
		snowShader(device, SHADER_PATH("SnowShaders/SnowShader.hlsl"), {}, ShaderType::GS | ShaderType::PS | ShaderType::VS)
	{
        srand(time(NULL));
     
	}

	SnowManager::~SnowManager()
	{
		
	}

	void SnowManager::updateSnow(float deltaTime, Camera * camera, ID3D11DeviceContext * context)
	{
    
        static float windTimer = 5000;
        static float windCounter = 0;
        static Vector3 randWindPrev(0, -1, 0);
        static Vector3 randWind(0, -1, 0);
        static float friction = 0.6f;

		//temp
        static auto ks = DirectX::Keyboard::KeyboardStateTracker();
        ks.Update(DirectX::Keyboard::Get().GetState());

		if (ks.pressed.P)
		{
			initializeSnowflakes(camera);
		}

        windCounter += deltaTime;
        if (windTimer <= windCounter)
        {
            randWindPrev = randWind;
            windCounter = 0;
            randWind.x = getRandomFloat(-1, 1);
            randWind.z = getRandomFloat(-1, 1);
            randWind.y = -1;
        }

		for (int i = 0; i < MAX_SNOW; i++)
		{
			if ((snowFlakes[i].position - camera->getPos()).Length() > SNOW_RADIUS)
				moveSnowFlake(camera, i);

			snowFlakes[i].distance = (snowFlakes[i].position - camera->getPos()).Length();
            snowFlakes[i].randomRot += getRandomFloat(0, ONE_DEG_IN_RAD * 5);

			snowFlakes[i].position += Vector3::Lerp(randWindPrev, randWind, windCounter / windTimer) * deltaTime * 0.01f;

		}
		
		snowBuffer.write(context, &snowFlakes[0], snowFlakeCount * sizeof(SnowFlake));

	}

	void SnowManager::addRandomSnowFlake(Camera * camera)
	{
		Vector3 randVec(getRandomFloat(-SNOW_RADIUS, SNOW_RADIUS), getRandomFloat(-SNOW_RADIUS, SNOW_RADIUS), getRandomFloat(-SNOW_RADIUS, SNOW_RADIUS));

		Vector3 finalPos = camera->getPos() + randVec;

		SnowFlake flake;
		flake.position = finalPos;
		flake.randomRot = getRandomFloat(0, PI * 2.f);
		flake.distance = randVec.Length();

		snowFlakes.push_back(flake);

		snowFlakeCount++;
	}

	void SnowManager::moveSnowFlake(Camera * camera, int snowFlake)
	{
        Vector3 randVec(getRandomFloat(-SNOW_RADIUS, SNOW_RADIUS), getRandomFloat(-SNOW_RADIUS, SNOW_RADIUS), getRandomFloat(-SNOW_RADIUS, SNOW_RADIUS));

        Vector3 finalPos = camera->getPos() + randVec;
        snowFlakes[snowFlake].position = camera->getPos() + finalPos;
	}

	//this function randomizes snow all over the frustum because otherwise all snow will start from the top
	void SnowManager::initializeSnowflakes(Camera * camera)
	{
		clearSnow();

		for (int i = 0; i < MAX_SNOW; i++)
		{
			addRandomSnowFlake(camera);
           
		}
	}

	void SnowManager::drawSnowflakes(ID3D11DeviceContext * context, Camera * camera, ID3D11RenderTargetView * target, DepthStencil * depthMap, SkyRenderer& sky)
	{
		context->GSSetConstantBuffers(0, 1, *camera->getBuffer());
		context->VSSetShaderResources(4, 1, snowBuffer);

        context->PSSetConstantBuffers(1, 1, *sky.getShaderBuffer());
        context->GSSetConstantBuffers(3, 1, *sky.getLightMatrixBuffer());
        context->PSSetShaderResources(3, 1, *sky.getDepthStencil());
        context->OMSetRenderTargets(1, &target, *depthMap);

        context->PSSetSamplers(1, 1, sky.getSampler());

        context->IASetInputLayout(nullptr);
		context->VSSetShader(snowShader, nullptr, 0);
		context->PSSetShader(snowShader, nullptr, 0);
		context->GSSetShader(snowShader, nullptr, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		
        context->Draw(snowFlakeCount, 0);

        ID3D11ShaderResourceView * nullSRV = nullptr;
        ID3D11RenderTargetView * nullRTV = nullptr;


        context->OMSetRenderTargets(1, &nullRTV, nullptr);
        context->PSSetShaderResources(3, 1, &nullSRV);
	}

	void SnowManager::recompile(ID3D11Device * device)
	{
		snowShader.recompile(device, SHADER_PATH("SnowShaders/SnowShader.hlsl"), {}, ShaderType::GS | ShaderType::PS | ShaderType::VS);
	}

	void SnowManager::clearSnow()
	{
		snowFlakes.clear();
		snowFlakeCount = 0;
	}
}