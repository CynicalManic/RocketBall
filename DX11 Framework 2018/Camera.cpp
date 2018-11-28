#include "Camera.h"



Camera::Camera(XMFLOAT4X4* view, XMFLOAT4X4*  projection, UINT windowWidth, UINT windowHeight)
{
	_windowHeight = windowHeight;
	_windowWidth = windowWidth;
	_view = view;
	_projection = projection;

}

void Camera::LookAt()
{
	XMStoreFloat4x4(_view, XMMatrixLookAtLH(_eye, _at, _up));
	XMStoreFloat4x4(_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, 0.01f, 100.0f));
}

void Camera::LookTo()
{
	XMVECTOR testVector = { 0,0,0 };
	if (XMVector3Equal(testVector, _at))
	{
		_at = { 0,0,1 };
	}

	XMStoreFloat4x4(_view, XMMatrixLookToLH(_eye, _at, _up));
	XMStoreFloat4x4(_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, 0.01f, 100.0f));
}

void Camera::Update(float t)
{

}

Camera::~Camera()
{
}
