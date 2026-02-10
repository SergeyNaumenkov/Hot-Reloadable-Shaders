/*

	Copyright 2026 Sergey Naumenkov

	File: Example.h
	Description: Demo app
	Note: ---

	Date: 10/02/2026

*/

#include "Example.h"

template<typename T>
inline void SafeRelease(T*& ptr) {
	if (ptr) {
		ptr->Release();
		ptr = nullptr;
	}
}

Example::Example()
{
	mRenderDevice = nullptr;
	mRenderDeviceContext = nullptr;
	mSwapChain = nullptr;
	mRenderTargetView = nullptr;
	mRenderContantBuffer = nullptr;

	g_World = {};
	g_View = {};
	g_Projection = {};

	mViewportWidth = 0;
	mViewportHeight = 0;

	mCurrentRenderWindow = nullptr;

	mHotReloadShaders = {};
	mShaderInformation = {};
}

Example::~Example()
{
	SafeRelease(mRenderDevice);
	SafeRelease(mRenderDeviceContext);
	SafeRelease(mSwapChain);
	SafeRelease(mRenderTargetView);
	SafeRelease(mRenderContantBuffer);
}

/// <summary>
/// Initialize DirectX 11
/// </summary>
/// <param name="wnd">Window for rendering</param>
/// <param name="initWidth">Window width</param>
/// <param name="initHeight">Window height</param>
/// <returns>if all created - true otherwise false</returns>
bool Example::Initialize(void* wnd, float initWidth, float initHeight)
{
	// Save viewport sizes
	mViewportWidth = initWidth;
	mViewportHeight = initHeight;

	// Save current window
	mCurrentRenderWindow = wnd;

	if (!CreateD3DDevices())
		return false;

	if (!CreateSwapChain())
		return false;

	if (!CreateRenderTargetView())
		return false;

	if (!CreateBuffers())
		return false;

	// Prepare scene
	Prepare();

	// Prepare Hot Reloadable Shaders
	PrepareHotReloadShaders();

	// tested
	LoadVertexShader();

	// Update viewport
	UpdateViewport(mViewportWidth, mViewportHeight);


	return true;
}

/// <summary>
/// Create devices for rendering
/// </summary>
/// <returns>true - if all created or false - if not created</returns>
bool Example::CreateD3DDevices()
{
	// In future I think need uses 11_1
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	int creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create device
	auto res = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, creationFlags,
		featureLevels, 1, D3D11_SDK_VERSION,
		&mRenderDevice,
		nullptr, &mRenderDeviceContext);

	if (FAILED(res))
	{
		printf("Failed create DirectX 11 devices!\n");
		return false;
	}

	return true;
}

/// <summary>
/// Create swapchain
/// </summary>
/// <returns>true - if all created or false - if not created</returns>
bool Example::CreateSwapChain()
{
	// Create device and swapchain
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2; // double buffering 
	swapDesc.BufferDesc.Width = (int)mViewportWidth;
	swapDesc.BufferDesc.Height = (int)mViewportHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = (HWND)mCurrentRenderWindow;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Get Factories
	IDXGIFactory* dxgiFactory = nullptr;
	IDXGIDevice* dxgiDevice = nullptr;

	mRenderDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	IDXGIAdapter* adapter = nullptr;
	dxgiDevice->GetAdapter(&adapter);
	adapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

	// Create swapchain
	auto res = dxgiFactory->CreateSwapChain(mRenderDevice, &swapDesc, &mSwapChain);
	if (FAILED(res))
	{
		printf("Failed create DirectX 11 SwapChain!\n");
		return false;
	}

	return true;
}

/// <summary>
/// Update viewport
/// </summary>
/// <param name="width">new width</param>
/// <param name="height">new height</param>
void Example::UpdateViewport(float width, float height)
{
	D3D11_VIEWPORT vp = {};
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;

	mRenderDeviceContext->RSSetViewports(1, &vp);
}

void Example::LoadVertexShader()
{
	// Load file
	ID3DBlob* code = nullptr;
	HRESULT hr = D3DCompileFromFile(
		L"VertexShader.hlsl", nullptr, nullptr, "main",
		"vs_5_0", 0, 0, &code, nullptr);

	if (FAILED(hr))
		return;

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	mRenderDevice->CreateInputLayout(layout, numElements, code->GetBufferPointer(), code->GetBufferSize(), &mRenderVertexLayout);

	// Set the input layout
	mRenderDeviceContext->IASetInputLayout(mRenderVertexLayout);
}

/// <summary>
/// Create buffers for rendering
/// </summary>
/// <returns>true - if all created or false - if not created</returns>
bool Example::CreateBuffers()
{
	DirectX::XMFLOAT4 redColor = DirectX::XMFLOAT4(1, 0, 0, 1);
	Vertex vertices[] =
	{
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f),  redColor },
		{ DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f),	 redColor },
		{ DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),	 redColor },
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f),	 redColor },
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), redColor },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f),  redColor },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f),	 redColor },
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f),  redColor },
	};

	// Describe dynamic vertex buffer
	D3D11_BUFFER_DESC buffer = {};
	buffer.ByteWidth = sizeof(Vertex) * 8;
	buffer.Usage = D3D11_USAGE_DYNAMIC;
	buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	// Create vertex buffer
	auto hr = mRenderDevice->CreateBuffer(&buffer, &InitData, &mRenderVertexBuffer);
	if (FAILED(hr))
	{
		printf("Failed create Vertex Buffer\n");
		return false;
	}

	// Describe dynamic index buffer
	D3D11_BUFFER_DESC indexBuffer = {};
	indexBuffer.ByteWidth = sizeof(UINT) * 36;
	indexBuffer.Usage = D3D11_USAGE_DYNAMIC;
	indexBuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// Describe index array
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		0,5,4,
		1,5,0,

		3,4,7,
		0,4,3,

		1,6,5,
		2,6,1,

		2,7,6,
		3,7,2,

		6,4,5,
		7,4,6,
	};

	buffer.Usage = D3D11_USAGE_DEFAULT;
	buffer.ByteWidth = sizeof(WORD) * 36;        
	buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer.CPUAccessFlags = 0;
	InitData.pSysMem = indices;

	// Create index buffer
	hr = mRenderDevice->CreateBuffer(&indexBuffer, &InitData, &mRenderIndexBuffer);
	if (FAILED(hr))
	{
		printf("Failed create Index Buffer\n");
		return false;
	}

	// Create constant buffer
	// Create the constant buffer
	buffer.Usage = D3D11_USAGE_DEFAULT;
	buffer.ByteWidth = sizeof(ConstantBuffer);
	buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer.CPUAccessFlags = 0;
	hr = mRenderDevice->CreateBuffer(&buffer, NULL, &mRenderContantBuffer);
	if (FAILED(hr))
		return false;

	return true;
}

/// <summary>
/// Create RTV
/// </summary>
/// <returns>true - if all created or false - if not created</returns>
bool Example::CreateRenderTargetView()
{
	// Get backbuffer
	ID3D11Texture2D* backBuffer = nullptr;
	auto res = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(res))
	{
		printf("Failed get backbuffer!\n");
		return false;
	}

	// Create render target view
	res = mRenderDevice->CreateRenderTargetView(backBuffer, nullptr, &mRenderTargetView);
	if (FAILED(res))
	{
		printf("Failed create Render Target View!\n");
		return false;
	}

	// Release
	backBuffer->Release();

	return true;
}

/// <summary>
/// Start frame
/// </summary>
void Example::StartFrame()
{
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // red,green,blue,alpha
	mRenderDeviceContext->ClearRenderTargetView(mRenderTargetView, ClearColor);
	mRenderDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, nullptr);

	// Start watching
	mHotReloadShaders.Start();
}

/// <summary>
/// Render Scene
/// </summary>
void Example::RenderScene()
{
	// Update our time
	static float t = 0.0f;
	static DWORD dwTimeStart = 0;
	DWORD dwTimeCur = GetTickCount();
	if (dwTimeStart == 0)
		dwTimeStart = dwTimeCur;
	t = (dwTimeCur - dwTimeStart) / 1000.0f;
	
	// rotating cube 
	g_World = DirectX::XMMatrixRotationY(t);

	// Update constant buffer and insert in to pipeline slot
	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);
	mRenderDeviceContext->UpdateSubresource(mRenderContantBuffer, 0, NULL, &cb, 0, 0);

	// Insert vertex and index buffers
	// Set index buffer
	mRenderDeviceContext->IASetIndexBuffer(mRenderIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	mRenderDeviceContext->IASetVertexBuffers(0, 1, &mRenderVertexBuffer, &stride, &offset);

	// Set primitive topology
	mRenderDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mRenderDeviceContext->VSSetConstantBuffers(0, 1, &mRenderContantBuffer);

	// Draw
	mRenderDeviceContext->DrawIndexed(36, 0, 0);
}

/// <summary>
/// End frame
/// </summary>
void Example::EndFrame()
{
	// Present frame
	mSwapChain->Present(0, 0);
}

/// <summary>
///  Prepare devices for rendering
/// </summary>
void Example::Prepare()
{
	// Initialize the world matrix
	g_World = DirectX::XMMatrixIdentity();

	// Initialize the view matrix
	DirectX::XMVECTOR Eye = DirectX::XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	DirectX::XMVECTOR At =  DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR Up =  DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_View = DirectX::XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	g_Projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, mViewportWidth / (FLOAT)mViewportHeight, 0.01f, 100.0f);
}

/// <summary>
/// Prepare hot reload hlsl shaders
/// </summary>
void Example::PrepareHotReloadShaders()
{
	// Add pixel shader
	mShaderInformation.bSaveToCSO = true; // Future 
	mShaderInformation.entryPoint = "main"; // Set entry point for shader
	mShaderInformation.hlslPath = "PixelShader.hlsl"; // Source file 
	mShaderInformation.isAutomationBind = false; // Allow system automate bind compiled shaders ( future )
	mShaderInformation.localName = "BasicPixelShader"; // Local name, right now not using
	mShaderInformation.localShaderType = HotReloadableShaderType::PixelShader; // Shader type ( in future added more )
	mShaderInformation.shaderVersion = "ps_5_0"; // shader version
	mShaderInformation.renderDevices.mRenderDevice = mRenderDevice; // d3d11 device ( for creation shaders )
	mShaderInformation.renderDevices.mRenderDeviceContext = mRenderDeviceContext; // d3d11 device ( for binding shaders )
	mHotReloadShaders.AddNewBundle(mShaderInformation); // add to bindle

	// Add vertex shader
	mShaderInformation.hlslPath = "VertexShader.hlsl"; // Source file 
	mShaderInformation.localName = "BasicVertexShader"; // Local name, right now not using
	mShaderInformation.shaderVersion = "vs_5_0"; // shader version
	mShaderInformation.localShaderType = HotReloadableShaderType::VertexShader; // Shader type ( in future added more )
	mHotReloadShaders.AddNewBundle(mShaderInformation); // add to bindle

	// Register callback
	mHotReloadShaders.ActionIfCompiled([this]() {
		//if (gHotReloadableShaders.IsCompiled()) <--- Not need, because callback called only when shaders compiled!

		// Get compiled shaders
		auto types = mHotReloadShaders.GetCompiledShadersType();
		for (auto& type : types)
		{
			// Bind shaders by shader type
			if (type.compiledShaderType == HotReloadableShaderType::VertexShader)
			{
				// For example
				auto shader = mHotReloadShaders.GetCompiledShaderByLocalName<ID3D11VertexShader*>("BasicVertexShader");
				mRenderDeviceContext->VSSetShader(shader, NULL, 0);
			}
			else if (type.compiledShaderType == HotReloadableShaderType::PixelShader)
			{
				// For example
				auto shader = mHotReloadShaders.GetCompiledShaderByLocalName<ID3D11PixelShader*>("BasicPixelShader");
				mRenderDeviceContext->PSSetShader(shader, NULL, 0);
			}
		}
		});
}
