#include "pch.h"
#include <UniDx/Light.h>

#include <UniDx/LightManager.h>


namespace UniDx{

void Light::OnEnable()
{
    LightManager::getInstance()->registerLight(this);
}


void Light::OnDisable()
{
    LightManager::getInstance()->unregisterLight(this);
}

}
