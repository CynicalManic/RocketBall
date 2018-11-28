#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "RotatingObject.h"
#include "Structures.h"
#include "Camera.h"


using namespace DirectX;

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pVertexBuffer2;
	ID3D11Buffer*           _pVertexBuffer3;
	ID3D11Buffer*           _pIndexBuffer;
	ID3D11Buffer*           _pIndexBuffer2;
	ID3D11Buffer*           _pIndexBuffer3;
	ID3D11Buffer*           _pConstantBuffer;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D*		_depthStencilBuffer;	
	ID3D11ShaderResourceView * _pTextureRV = nullptr;
	ID3D11SamplerState *	_pSamplerLinear = nullptr;
	// Blending
	ID3D11BlendState*		_transparency;
	// world matrices
	XMFLOAT4X4              _world [10];
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;
	// diffuse and ambient lighting variables
	XMFLOAT4				_ambientMaterial;
	XMFLOAT4				_ambientLight;
	XMFLOAT3				_lightDirection;
	XMFLOAT4				_diffuseMaterial;
	XMFLOAT4				_diffuseLight;
	// specular variables
	XMFLOAT4				_specularMtrl;
	XMFLOAT4				_specularLight;
	float					_specularPower;
	XMFLOAT3				_eyePosW;
	// object variables
	int						_objectNumber = 10;
	RotatingObject*			_objects [10];
	// Camera Variables	
	Camera*					_camera [2];
	UINT					_activeCamera = 0;
	// Time Variables
	float					_oldTime = 0;
	



private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

