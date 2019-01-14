#include "RotatingObject.h"

RotatingObject::RotatingObject()
{
	
}
RotatingObject::~RotatingObject()
{
	
}


void RotatingObject::Update(float t)
{
	XMStoreFloat4x4(_world, 
		XMMatrixRotationX((t * _rotationSpeed[0])) * XMMatrixRotationY(t * _rotationSpeed[1]) * XMMatrixRotationZ((t * _rotationSpeed[2]))
		* XMMatrixTranslation(_worldPosition[0], _worldPosition[1], _worldPosition[2])
		* XMMatrixRotationX((t * _orbitSpeed[0])) * XMMatrixRotationY(t * _orbitSpeed[1]) * XMMatrixRotationZ((t * _orbitSpeed[2])));
}


