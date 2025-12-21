#include "pch.h"
#include <UniDx/Input.h>

namespace UniDx{

std::unique_ptr<DirectX::Keyboard> Input::keyboard;
DirectX::Keyboard::State Input::nowKeyState;
DirectX::Keyboard::State Input::prevKeyState;

}
