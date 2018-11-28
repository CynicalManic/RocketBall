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
	XMVECTOR test = { 0,0,0 };
	test = { 0,0,1 };

	

	XMStoreFloat4x4(_view, XMMatrixLookToLH(_eye, test, _up));
	XMStoreFloat4x4(_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, 0.01f, 100.0f));
}

Camera::~Camera()
{
}
