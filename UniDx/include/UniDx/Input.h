#pragma once

#include <Keyboard.h>
#include "UniDxDefine.h"


namespace UniDx
{

using DirectX::Keyboard;


// Input情報
class Input
{
public:
    static void initialize()
    {
        keyboard = std::make_unique<Keyboard>();
    }

    static void update()
    {
        prevKeyState = nowKeyState;
        nowKeyState = keyboard->GetState();
    }

    static bool GetKey(Keyboard::Keys key)
    {
        return nowKeyState.IsKeyDown(key);
    }

    static bool GetKeyDown(Keyboard::Keys key)
    {
        return prevKeyState.IsKeyUp(key) && nowKeyState.IsKeyDown(key);
    }

    static bool GetKeyUp(Keyboard::Keys key)
    {
        return prevKeyState.IsKeyDown(key) && nowKeyState.IsKeyUp(key);
    }

protected:
    static std::unique_ptr<Keyboard> keyboard;
    static Keyboard::State nowKeyState;
    static Keyboard::State prevKeyState;
};

}
