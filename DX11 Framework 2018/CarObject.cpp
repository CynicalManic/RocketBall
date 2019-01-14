#include "CarObject.h"



CarObject::CarObject()
{

}


CarObject::~CarObject()
{
}

void CarObject::Update(float t)
{
	XMStoreFloat4x4(_world, XMMatrixTranslation(_worldPosition[0], _worldPosition[1], _worldPosition[2]) * 
		XMMatrixScaling(0.1f, 0.1f, 0.1f));
}
 