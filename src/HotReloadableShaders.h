/*

	Copyright 2026 Sergey Naumenkov

	File: HotReloadableShaders.h
	Description: Debug tool for hot reload .hlsl files
	Note: When user is edit and save .hlsl, tool is open and read file and compile
		  After compiling(if successful) - updated shaders, and (if user is set option) bind new shaders

	Date: 10/02/2026

*/

#ifndef HotReloadableShades_h
#define HotReloadableShades_h

#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <map>

#include <d3d11.h>
#include <d3dcompiler.h>

enum class HotReloadableShaderType
{
	VertexShader,
	PixelShader
};

struct CompiledQueue
{
	HotReloadableShaderType compiledShaderType;
};

/// <summary>
/// Hot reloadable shader information
/// </summary>
struct ShaderInformation
{
	// Local name for directly retrieving shader information data
	const char* localName;

	// local shader type
	HotReloadableShaderType localShaderType;

	// Shader type
	// ps_(version)/vs_(version)
	const char* shaderVersion;

	// Entry point in shaders
	// Default: main 
	const char* entryPoint;

	// Path to .hlsl files
	const char* hlslPath;

	// Allow system after compilation to save updated shaders.
	bool bSaveToCSO;

	// Allow system after compilation to bind Your shaders
	// *Note* for this function your must pass your ID3D11Pixel(Vertex)Shaders
	bool isAutomationBind;

	// Last time when file is be changed
	int mLastWriteTime;

	// Render devices
	struct D3DRenderDevices
	{
		ID3D11Device* mRenderDevice;
		ID3D11DeviceContext* mRenderDeviceContext;
	};

	// Render devices
	D3DRenderDevices renderDevices;
};

class HotReloadableShades
{
public:
	HotReloadableShades();
	~HotReloadableShades();

	// Add new shader information
	void AddNewBundle(ShaderInformation& information);

	// Start watching
	void Start();

	// Bring system get Shader information data
	ShaderInformation* GetShaderInformationByLocalName(const char* localName);

	// Have the shaders been compiled
	bool IsCompiled();

	// Get compiled shaders type
	std::vector<CompiledQueue> GetCompiledShadersType();

	// Get compiled pixel shader 
	ID3D11PixelShader* GetCompiledPixelShader();

	// Get compiled pixel shader 
	ID3D11VertexShader* GetCompiledVertexShader();

	// Set custom callback, which called when shaders is compiled
	void ActionIfCompiled(std::function<void()> callback);

protected:

	// Watch for files
	void StartWatch();

	// Compile file
	bool CompileFile(ShaderInformation& info);

	// Create pixel shader
	bool CreatePixelShader(ShaderInformation& info, ID3DBlob* shader);

	// Create vertex shader
	bool CreateVertexShader(ShaderInformation& info, ID3DBlob* shader);

private:
	std::vector<ShaderInformation> mShadersInformation;
	std::vector<CompiledQueue> mCompiledShaders;
	std::map<const char*, unsigned long long> mTimeChanged;

	bool bIsCompiled;
	ID3D11PixelShader* mCompiledPixelShader;
	ID3D11VertexShader* mCompiledVertexShader;

	std::function<void()> mCustomCallbackWhenShadersIsCompiled;
};

/// <summary>
/// Constructor
/// </summary>
inline HotReloadableShades::HotReloadableShades()
{
	bIsCompiled = false;

	mCompiledPixelShader = nullptr;
	mCompiledVertexShader = nullptr;
}

/// <summary>
/// Destructor
/// </summary>
inline HotReloadableShades::~HotReloadableShades()
{
	if (mCompiledPixelShader)
	{
		mCompiledPixelShader->Release();
		mCompiledPixelShader = nullptr;
	}

	if (mCompiledVertexShader)
	{
		mCompiledVertexShader->Release();
		mCompiledVertexShader = nullptr;
	}
}

/// <summary>
/// Add new shader information
/// </summary>
/// <param name="information">Information with your shaders which is enable hot reload</param>
inline void HotReloadableShades::AddNewBundle(ShaderInformation& information)
{
	mShadersInformation.push_back(information);
	mTimeChanged[information.localName] = 0;
}

/// <summary>
/// Start watching
/// </summary>
inline void HotReloadableShades::Start()
{
	// Watching 
	StartWatch();
}

/// <summary>
/// Bring system get Shader information data
/// </summary>
/// <param name="localName">local name of shader information</param>
/// <returns>Shader information</returns>
inline ShaderInformation* HotReloadableShades::GetShaderInformationByLocalName(const char* localName)
{
	// try find shader information
	for (auto& info : mShadersInformation)
	{
		if (!strcmp(info.localName, localName))
			return &info;
	}

	return nullptr;
}

/// <summary>
/// Have the shaders been compiled
/// </summary>
/// <returns></returns>
inline bool HotReloadableShades::IsCompiled()
{
	return bIsCompiled;
}

/// <summary>
/// Get compiled shaders type
/// </summary>
/// <returns></returns>
inline std::vector<CompiledQueue> HotReloadableShades::GetCompiledShadersType()
{
	return mCompiledShaders;
}

/// <summary>
/// Get compiled pixel shader 
/// </summary>
/// <returns></returns>
inline ID3D11PixelShader* HotReloadableShades::GetCompiledPixelShader()
{
	return mCompiledPixelShader;
}

/// <summary>
/// Get compiled pixel shader 
/// </summary>
/// <returns></returns>
inline ID3D11VertexShader* HotReloadableShades::GetCompiledVertexShader()
{
	return mCompiledVertexShader;
}

/// <summary>
/// Set custom callback, which called when shaders is compiled
/// </summary>
/// <param name="callback">Callbacl</param>
inline void HotReloadableShades::ActionIfCompiled(std::function<void()> callback)
{
	mCustomCallbackWhenShadersIsCompiled = callback;
}

/// <summary>
/// Get FILETIME in unsigned long long
/// </summary>
/// <param name="ft">FILETIME data</param>
/// <returns>unsigned long long</returns>
inline unsigned long long FileTimeToUInt64(const FILETIME& ft) {
	ULARGE_INTEGER uli;
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	return uli.QuadPart;
}

/// <summary>
/// Watch for files
/// </summary>
inline void HotReloadableShades::StartWatch()
{
	bIsCompiled = false;
	mCompiledShaders.clear();

	// Monitor every file that may change.
	for (auto& info : mShadersInformation)
	{
		WIN32_FIND_DATAA findData;
		auto handle = FindFirstFileA(info.hlslPath, &findData);
		if (handle == INVALID_HANDLE_VALUE)
		{
			// Moved, Changed, Renamed, Deleted?
			continue;
		}
		FindClose(handle);

		auto time = FileTimeToUInt64(findData.ftLastWriteTime);
		if (mTimeChanged[info.localName] != time)
		{
			CompileFile(info);

			// Update last time 
			mTimeChanged[info.localName] = time;
		}
	}

	// if callback is set
	// Call it
	if (mCustomCallbackWhenShadersIsCompiled && IsCompiled())
	{
		mCustomCallbackWhenShadersIsCompiled();
	}
}

/// <summary>
/// Read file 
/// </summary>
/// <param name="filename">Path to file</param>
/// <param name="buffer">out buffer</param>
/// <returns></returns>
inline bool ReadFile(const char* filename, std::vector<unsigned char>& buffer)
{
	// sometimes text editors may not have time to update the file,
	// in which case the buffer will be empty. 
	// Try to read it again.
	int maxAttempts = 5;
	int attempts = 0;
	for (int i = 0; i < maxAttempts; i++)
	{
		// Open file
		HANDLE hFile = CreateFileA(
			filename,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // set all mode, because all most code editors opened file with all mode.
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (hFile == INVALID_HANDLE_VALUE) {
			return false;
		}

		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(hFile, &fileSize)) {
			CloseHandle(hFile);
			return false;
		}

		// if size = 0;
		// the file did not have time to update
		if (fileSize.QuadPart == 0)
		{
			attempts++;
			continue;
		}

		buffer.resize(fileSize.QuadPart);
		DWORD bytesRead = 0;
		if (!ReadFile(hFile, buffer.data(), (DWORD)fileSize.QuadPart, &bytesRead, NULL)) {
			CloseHandle(hFile);
			return false;
		}

		CloseHandle(hFile);

		return true;
	}

	printf("File <%s> It can't be read, check the editor where you're editing the file, there might be a problem with it. Attempt[%i]\"", filename, attempts);

	return false;
}

/// <summary>
/// Compile file
/// </summary>
/// <param name="filePath">.hlsl path</param>
/// <param name="info">Shader information</param>
/// <returns>bool is compiled otherwise false</returns>
inline bool HotReloadableShades::CompileFile(ShaderInformation& info)
{
	std::vector<unsigned char> fileBuffer;

	bool isDone = ReadFile(info.hlslPath, fileBuffer);
	if (!isDone)
		return false;

	// Compile shader
	ID3DBlob* shader = nullptr;
	ID3DBlob* error = nullptr;
	auto hr = D3DCompile(fileBuffer.data(), fileBuffer.size(), nullptr, nullptr, nullptr, info.entryPoint, info.shaderVersion, 0, 0, &shader, &error);
	if (FAILED(hr))
	{
		if (error)
		{
			printf("Failed compile <%s> error message:\n%s", info.hlslPath, (const char*)error->GetBufferPointer());
			error->Release();
			error = nullptr;
		}
		return false;
	}

	// Create compiled shaders
	if (info.localShaderType == HotReloadableShaderType::VertexShader)
	{
		auto isCreated = CreateVertexShader(info, shader);
		if (!isCreated)
			return false;
	}
	else if (info.localShaderType == HotReloadableShaderType::PixelShader)
	{
		auto isCreated = CreatePixelShader(info, shader);
		if (!isCreated)
			return false;
	}

	// Release buffers
	fileBuffer.clear();

	return true;
}

/// <summary>
/// Create pixel shader
/// </summary>
/// <param name="info">Shader information</param>
/// <param name="shader">shader blob</param>
/// <returns></returns>
inline bool HotReloadableShades::CreatePixelShader(ShaderInformation& info, ID3DBlob* shader)
{
	if (mCompiledPixelShader)
	{
		mCompiledPixelShader->Release();
		mCompiledPixelShader = nullptr;
	}

	// Create
	auto res = info.renderDevices.mRenderDevice->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mCompiledPixelShader);
	if (FAILED(res))
	{
		shader->Release();
		return false;
	}

	mCompiledShaders.push_back({ HotReloadableShaderType::PixelShader });
	bIsCompiled = true;
	return true;
}

/// <summary>
/// Create vertex shader
/// </summary>
/// <param name="info">Shader information</param>
/// <param name="shader">shader blob</param>
/// <returns></returns>
inline bool HotReloadableShades::CreateVertexShader(ShaderInformation& info, ID3DBlob* shader)
{
	if (mCompiledVertexShader)
	{
		mCompiledVertexShader->Release();
		mCompiledVertexShader = nullptr;
	}

	// Create
	auto res = info.renderDevices.mRenderDevice->CreateVertexShader(shader->GetBufferPointer(), shader->GetBufferSize(), nullptr, &mCompiledVertexShader);
	if (FAILED(res))
	{
		shader->Release();
		return false;
	}

	mCompiledShaders.push_back({ HotReloadableShaderType::VertexShader });
	bIsCompiled = true;
	return true;
}


#endif // !HotReloadableShades_h
