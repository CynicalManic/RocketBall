#include "Application.h"
#include <time.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pVertexBuffer2 = nullptr;
	_pIndexBuffer = nullptr;
	_pIndexBuffer2 = nullptr;
	_pConstantBuffer = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}
	
    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

	// Light direction from surface (XYZ)
	_lightDirection = XMFLOAT3(0.25f, 0.75, -1.0f);
	// Diffuse material properties (RGBA)
	_diffuseMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	// Diffuse light colour (RGBA)
	_diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	_ambientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f,0.0f);
	_ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
	
    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize world objects
	for (int i = 0; i < 1; i++)
	{
		_objects[i] = new RotatingObject();
		_objects[i]->SetWorldPosition({ 0,0,0 });
		_objects[i]->SetIndexBuffer(_pIndexBuffer);
		_objects[i]->SetVertexBuffer(_pVertexBuffer);
		_objects[i]->SetIndexCount(36);
		_objects[i]->SetMatrix(&_world[0]);
		_objects[i]->SetRotationSpeed({ 1,1,1 });
	}

	for (int i = 1; i < _objectNumber; i++)
	{
		_objects[i] = new RotatingObject();
		//_objects[i]->SetWorldPosition({ 1,0,0 });
		_objects[i]->SetIndexBuffer(_pIndexBuffer2);
		_objects[i]->SetVertexBuffer(_pVertexBuffer2);
		_objects[i]->SetIndexCount(18);
		_objects[i]->SetMatrix(&_world[i]);
		_objects[i]->SetRotationSpeed({ 1,1,1 });
		_objects[i]->SetOrbitSpeed({ 0,1,0 });
	}
	

	srand(time(NULL));
	for (int i = 1; i < _objectNumber; i++)
	{
		_objects[i]->SetWorldPosition({ ((((float)rand()) / RAND_MAX) * 10) - 5 , 0 , ((((float)rand()) / RAND_MAX) * 10) - 5 });
	}
	

	// Initialize the world matrix
	//XMStoreFloat4x4(&_world[0], XMMatrixIdentity());
	//XMStoreFloat4x4(&_world[1], XMMatrixIdentity());
	

    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	_camera2 = new Camera(&_view, &_projection, _WindowWidth, _WindowHeight);
	_camera2->SetEye(Eye);
	_camera2->LookAt();

	_camera = new Camera(&_view, &_projection, _WindowWidth, _WindowHeight);

	_camera2->LookAt();
	_camera->LookTo();



	//XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));
	//XMStoreFloat4x4(&_view, XMMatrixLookAtLH(_camera->GetEye, _camera->GetUp, _camera->GetAt));

	_specularMtrl = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	_specularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	_specularPower = 3.0f;
	_eyePosW = XMFLOAT3(0.0f, 2.0f, -3.0f);

    // Initialize the projection matrix
	//XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));

	// Setup Texturing
	CreateDDSTextureFromFile(_pd3dDevice, L"Crate_COLOR.dds", nullptr, &_pTextureRV);
	_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
		// Back Face
        { XMFLOAT3(	0.5f, -0.5f, 0.5f), XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(0,1) },	// Back Top Right
        { XMFLOAT3( -0.5f, -0.5f, 0.5f), XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(0,0) }, // Back Top Left			
        { XMFLOAT3( 0.5f, 0.5f, 0.5f), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(1,1) },		// Back Bottom Right
        { XMFLOAT3(	-0.5f, 0.5f, 0.5f), XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT2(1,0) },	// Back Bottom Left		
		// 0,1,2
		// 2,1,3

		//Front Face
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0,0) },// Front Top Left		
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(1,0) },	// Front Top Right		
		{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT2(0,1) },	// Front Bottom Left
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(1,1) },	// Front Bottom Right
		// 4,5,6
		// 6,5,7

		// Left Face
		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(0,0) }, // Back Top Left			
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(1,0) },// Front Top Left		
		{ XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT2(1,1) },	// Back Bottom Left
		{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT2(0,1) },	// Front Bottom Left
		// 8,9,10
		// 10,9,11

		// Right Face
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(0,0) },	// Front Top Right		
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(1,0) },	// Back Top Right		
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(0,1) },	// Front Bottom Right
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(1,1) },		// Back Bottom Right
		// 12,13,14
		// 14,13,15

		// Top Face
		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(0,0) }, // Back Top Left			
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT2(1,0) },	// Back Top Right		
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0,1) },// Front Top Left
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(1,1) },	// Front Top Right
		// 16,17,18
		// 18,17,19

		// Bottom Face
		{ XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT3(0.5f, 0.5f, -0.5f), XMFLOAT2(0,0) },	// Front Bottom Right	
		{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT2(1,0) },	// Front Bottom Left	
		{ XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT2(0,1) },		// Back Bottom Right
		{ XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT3(-0.5f, 0.5f, 0.5f), XMFLOAT2(1,1) },	// Back Bottom Left
		// 20,21,22
		// 22,21,23
    };

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 36;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

	SimpleVertex triangleVertices[] =
	{
		{ XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT2(0,0) },
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-0.5f, -0.5f, -0.5f),  XMFLOAT2(1,0) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(0.5f, -0.5f, -0.5f),  XMFLOAT2(0,1) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT3(-0.5f, -0.5f, 0.5f),  XMFLOAT2(1,1) },
		{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT3(0.5f, -0.5f, 0.5f),  XMFLOAT2(0,0) },
	};

	bd.ByteWidth = sizeof(SimpleVertex) * 5;
	InitData.pSysMem = triangleVertices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer2);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
		// back
		2, 1, 0,
		3, 1, 2,
		// front
		6, 5, 4,
		7, 5, 6,
		// left
		10, 9, 8,
		11, 9, 10,
		// right
		14, 13, 12,
		15, 13, 14,
		// top
		18, 17, 16,
		19, 17, 18,
		// bottom
		20, 21, 22,
		22, 21, 23,
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

	WORD triangleIndices[] =
	{
		// back
		0,2,1,
		0,4,2,
		0,3,4,
		0,1,3,
		4,1,2,
		3,1,4,
	};

	bd.ByteWidth = sizeof(WORD) * 18;
	InitData.pSysMem = triangleIndices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer2);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();

	InitIndexBuffer();    

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

	// Blending Setup
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	_pd3dDevice->CreateBlendState(&blendDesc, &_transparency);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();


    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

	//&_world.gTime = t;

    //
    // Animate the cube
    //
	
	for (int i = 0; i < _objectNumber; i++)
	{
		_objects[i]->Update(t);
	}	
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	XMMATRIX world  = XMLoadFloat4x4(&_world[0]);
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);
    //
    // Update variables
    //
    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);

	cb.SpecularMtrl = _specularMtrl;
	cb.SpecularLight = _specularLight;
	cb.SpecularPower = _specularPower;
	cb.EyePosW = _eyePosW;

	cb.AmbientMaterial = _ambientMaterial;
	cb.AmbientLight = _ambientLight;	
	cb.LightVecW = _lightDirection;
	cb.DiffuseMtrl = _diffuseMaterial;
	cb.DiffuseLight = _diffuseLight;

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

	// "fine-tune" the blending equation
	//float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };
	float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };

	// Set the default blend state (no blending) for opaque objects
	_pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	// Render opaque objects //
      	
	for (int i = 1; i < _objectNumber; i++)
	{
		_objects[i]->Draw(_pImmediateContext, _pConstantBuffer, &world, &cb);
	}
	// Set the blend state for transparent objects
	_pImmediateContext->OMSetBlendState(_transparency, blendFactor, 0xffffffff);
	
	for (int i = 0; i < 1; i++)
	{
		_objects[i]->Draw(_pImmediateContext, _pConstantBuffer, &world, &cb);
	}

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}