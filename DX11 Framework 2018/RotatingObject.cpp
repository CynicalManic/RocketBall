#include "RotatingObject.h"

RotatingObject::RotatingObject()
{
	
}
RotatingObject::~RotatingObject()
{
	
}
void RotatingObject::Initialise(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, XMFLOAT4X4 world, UINT indexCount)
{
	_vertexBuffer = vertexBuffer;
	_indexBuffer = indexBuffer;
	_world = &world;
	_indexCount = indexCount;
}

void RotatingObject::Draw(ID3D11DeviceContext* _pImmediateContext, ID3D11Buffer* _pConstantBuffer, XMMATRIX* mWorld, ConstantBuffer* cb)
{
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	_pImmediateContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	_pImmediateContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	*mWorld = XMLoadFloat4x4(_world);
	cb->mWorld = XMMatrixTranspose(*mWorld);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, cb, 0, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->DrawIndexed(_indexCount, 0, 0);
}

void RotatingObject::Update(float t)
{
	XMStoreFloat4x4(_world, 
		XMMatrixRotationX((t * _rotationSpeed[0])) * XMMatrixRotationY(t * _rotationSpeed[1]) * XMMatrixRotationZ((t * _rotationSpeed[2]))
		* XMMatrixTranslation(_worldPosition[0], _worldPosition[1], _worldPosition[2])
		* XMMatrixRotationX((t * _orbitSpeed[0])) * XMMatrixRotationY(t * _orbitSpeed[1]) * XMMatrixRotationZ((t * _orbitSpeed[2])));
}


