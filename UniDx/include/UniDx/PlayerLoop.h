/**
 * @file PlayerLoop.h
 * @brief フレームワーク全体のループ処理を行うクラス
 */
#pragma once

#include <windows.h>
#include <Keyboard.h>

#include "Singleton.h"

namespace UniDx
{

class GameObject;
class Camera;
class Canvas;

/**
 * @brief フレームワーク全体のループ処理を行うクラス。
 * Unityでは実行リストが作られるが、簡略化のため、全てのGameObjectとComponentを巡回して実行する。
 */
class PlayerLoop : public Singleton<PlayerLoop>
{
public:
    /**
     * @brief プレイヤーループの初期化。
     * @param CreateWindowW()で生成するウィンドウハンドル
     */
    virtual void Initialize(HWND hWnd);

    /// @brief ゲーム全体のメインループ
    virtual int MainLoop();

    void ProcessKeyboardMessage(UINT message, WPARAM wParam, LPARAM lParam)
    {
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
    }

    void registerCanvas(Canvas* c);
    void unregisterCanvas(Canvas* c);

protected:
    virtual void fixedUpdate();
    virtual void physics();
    virtual void input();
    virtual void update();
    virtual void lateUpdate();
    virtual void render();
    virtual void checkDestroy();
    virtual void finalize();

    void awake(GameObject* object);
    void fixedUpdate(GameObject* object);
    void checkStart(GameObject* object);
    void update(GameObject* object);
    void lateUpdate(GameObject* object);
    void render(GameObject* object, const Camera& camera);

private:
    std::vector<Canvas*> canvas_;

    void createScene();
};

}