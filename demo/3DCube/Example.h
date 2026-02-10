/*

	Copyright 2026 Sergey Naumenkov

	File: Example.h
	Description: Demo app
	Note: ---

	Date: 10/02/2026

*/

#ifndef Example_h
#define Example_h
#include <DirectXMath.h>

#include "HotReloadableShaders.h"

struct Vertex
{
	DirectX::XMFLOAT3 mPos;
	DirectX::XMFLOAT4 mColor;
};

struct ConstantBuffer
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProjection;
};

class Example
{
public:
	Example();
	~Example();

	// Initialize DirectX 11
	bool Initialize(void* wnd, float initWidth, float initHeight);

	// Create devices for rendering
	bool CreateD3DDevices();

	// Create swapchain
	bool CreateSwapChain();

	// Update viewport
	void UpdateViewport(float width, float height);

	// Create buffers for rendering
	bool CreateBuffers();

	// Create RTV
	bool CreateRenderTargetView();

	// Start frame
	void StartFrame();

	// Render Scene
	void RenderScene();

	// End frame
	void EndFrame();

protected:

	// Only for test
	void LoadVertexShader();

	// Prepare devices for rendering
	void Prepare();

	// Prepare hot reload hlsl shaders
	void PrepareHotReloadShaders();

private:
	ID3D11Device* mRenderDevice;
	ID3D11DeviceContext* mRenderDeviceContext;
	IDXGISwapChain* mSwapChain;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Buffer* mRenderVertexBuffer;
	ID3D11Buffer* mRenderIndexBuffer;
	ID3D11Buffer* mRenderContantBuffer;

	ID3D11InputLayout* mRenderVertexLayout;

	// Matrices
	DirectX::XMMATRIX g_World;
	DirectX::XMMATRIX g_View;
	DirectX::XMMATRIX g_Projection;

	float mViewportWidth;
	float mViewportHeight;
	void* mCurrentRenderWindow;

	HotReloadableShaders mHotReloadShaders;
	ShaderInformation mShaderInformation;
};

#endif Example_h