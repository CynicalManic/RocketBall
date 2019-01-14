#pragma once
#include "ObjectClass.h"

using namespace DirectX;

class RotatingObject : public ObjectClass
{
protected:
	std::array<float, 3> _rotationSpeed = { 0,0,0 };
	std::array<float, 3> _orbitSpeed = { 0,0,0 };
private:
	
public:
	RotatingObject();
	~RotatingObject();
	void Update(float t);
	void SetRotationSpeed(std::array<float, 3> rotationSpeed) { _rotationSpeed = rotationSpeed; };
	void SetOrbitSpeed(std::array<float, 3> orbitSpeed) { _orbitSpeed = orbitSpeed; };
};