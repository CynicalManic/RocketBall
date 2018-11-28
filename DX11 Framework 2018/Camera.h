#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <array>
#include "Structures.h"

using namespace DirectX;

class Camera
{
protected:
	XMVECTOR _eye = XMVectorSet(0.0f, 2.0f, -3.0f, 0.0f);
	XMVECTOR _at = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR _up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMFLOAT4X4* _view;
	XMFLOAT4X4*  _projection;
	UINT		_windowHeight;
	UINT		_windowWidth;
public:
	Camera(XMFLOAT4X4* view, XMFLOAT4X4* projection, UINT _windowWidth, UINT _windowHeight);
	~Camera();
	XMVECTOR GetEye() { return _eye; };
	XMVECTOR GetAt() { return _eye; };
	XMVECTOR GetUp() { return _eye; };
	void SetEye(XMVECTOR eye) { _eye = eye; };
	void SetAt(XMVECTOR at) { _at = at; };
	void SetUp(XMVECTOR up) { _up = up; };
	void LookAt();
	void LookTo();
	void Update(float t);
};

