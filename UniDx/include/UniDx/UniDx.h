#pragma once

#include "UniDxDefine.h"
#include "Math.h"
#include "Debug.h"
#include "Func.h"

#include "GameObject.h"
#include "Transform.h"
#include "GameObject_impl.h"
#include "Random.h"
#include "Behaviour.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "Camera.h"
#include "Light.h"


namespace UniDx
{

std::string ToUtf8(const wstring& wstr);
wstring ToUtf16(const std::string str);

}
