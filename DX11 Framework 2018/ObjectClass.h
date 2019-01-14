#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <array>
#include "Structures.h"

class ObjectClass
{
protected:
	ID3D11Buffer * _vertexBuffer;
	ID3D11Buffer* _indexBuffer;
	XMFLOAT4X4*    _world;
	UINT		_indexCount;
	std::array<float, 3> _worldPosition = { 0,0,0 };
public:
	ObjectClass();
	~ObjectClass();

	void Initialise(ID3D11Buffer* _vertexBuffer, ID3D11Buffer* _indexBuffer, XMFLOAT4X4 _world, UINT _indexCount);
	void SetVertexBuffer(ID3D11Buffer* vertexBuffer) { _vertexBuffer = vertexBuffer; };
	void SetIndexBuffer(ID3D11Buffer* indexBuffer) { _indexBuffer = indexBuffer; };
	void SetMatrix(XMFLOAT4X4* world) { _world = world; };
	void SetIndexCount(UINT indexCount) { _indexCount = indexCount; };
	void SetWorldPosition(std::array<float, 3> worldPosition) { _worldPosition = worldPosition; };
	void LoadMesh(MeshData _data);
	virtual void Draw(ID3D11DeviceContext* _pImmediateContext, ID3D11Buffer* _pConstantBuffer, XMMATRIX* mWorld, ConstantBuffer* cb);
	virtual void Update(float t);
	XMFLOAT4X4* GetMatrix() { return _world; };
};

