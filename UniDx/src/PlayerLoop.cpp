#include "pch.h"
#include <UniDx/PlayerLoop.h>

#include <string>
#include <chrono>

#include <Keyboard.h>          // DirectXTK
using namespace DirectX;

// DirectXテクスチャライブラリを使用できるようにする
#include <DirectXTex.h>

// DirextXフォントライブラリを使用できるようにする
#include <SpriteFont.h>

#include <UniDx/D3DManager.h>
#include <UniDx/Time.h>
#include <UniDx/SceneManager.h>
#include <UniDx/Scene.h>
#include <UniDx/Behaviour.h>
#include <UniDx/Camera.h>
#include <UniDx/Renderer.h>
#include <UniDx/Physics.h>
#include <UniDx/LightManager.h>
#include <UniDx/Input.h>
#include <UniDx/Canvas.h>

using namespace std;
using namespace UniDx;

namespace UniDx
{

// -----------------------------------------------------------------------------
//   Initialize(HWND hWnd)
// -----------------------------------------------------------------------------
void PlayerLoop::Initialize(HWND hWnd)
{
    // Direct3Dインスタンス作成
    D3DManager::create();

    // Direct3D初期化
    D3DManager::getInstance()->Initialize(hWnd, 1280, 720);

    // シーンマネージャのインスタンス作成
    SceneManager::create();

    // 入力の初期化
    Input::initialize();

    // 物理エンジンのインスタンス作成
    Physics::create();

    // ライトマネージャのインスタンス作成
    LightManager::create();
}


// -----------------------------------------------------------------------------
// 初期シーン作成
// -----------------------------------------------------------------------------
void PlayerLoop::createScene()
{
    SceneManager::getInstance()->createScene();

    // Awake
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        awake(&*it);
    }
}


// -----------------------------------------------------------------------------
// ゲーム全体のメインループ
// -----------------------------------------------------------------------------
int PlayerLoop::MainLoop()
{
    MSG msg;

    Time::Start();
    double restFixedUpdateTime = 0.0f;

    // デフォルトのシーン作成
    createScene();

    // メイン メッセージ ループ:
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            //============================================
            // ウィンドウメッセージ処理
            //============================================
            // 終了メッセージがきた
            if (msg.message == WM_QUIT) {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        using clock = std::chrono::steady_clock;          // モノトニックなので経過時間計測向き
        auto start = clock::now();

        //============================================
        // ゲームの処理を書く
        //============================================
        // 画面を塗りつぶす
        D3DManager::getInstance()->Clear(0.3f, 0.5f, 0.9f, 1.0f);

        Time::SetDeltaTimeFixed();

        while (restFixedUpdateTime > Time::fixedDeltaTime)
        {
            // 固定時間更新更新
            fixedUpdate();

            // 物理計算
            physics();

            restFixedUpdateTime -= Time::fixedDeltaTime;
        }

        Time::SetDeltaTimeFrame();

        // 入力更新
        input();

        // 更新処理
        update();

        // 後更新処理
        lateUpdate();

        // 描画処理
        render();

        // バックバッファの内容を画面に表示
        D3DManager::getInstance()->Present();

        // 時間計算
        double deltaTime = std::chrono::duration<double>(clock::now() - start).count();
        restFixedUpdateTime += deltaTime;

        Time::UpdateFrame(deltaTime);
    }

    // 終了処理
    finalize();

    return (int)msg.wParam;
}


// 固定時間更新更新
void PlayerLoop::fixedUpdate()
{
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        fixedUpdate(&*it);
    }
}


// 物理計算
void PlayerLoop::physics()
{
    Physics::getInstance()->simulatePositionCorrection(Time::fixedDeltaTime);
}


// 入力更新
void PlayerLoop::input()
{
    Input::update();
}


//  更新処理
void PlayerLoop::update()
{
    // 各コンポーネントの Start()
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        checkStart(&*it);
    }

    // 各コンポーネントの Update()
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        update(&*it);
    }
}


// 後の更新処理
void PlayerLoop::lateUpdate()
{
    // 各コンポーネントの LateUpdate()
    for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
    {
        lateUpdate(&*it);
    }
}


// 画面の描画処理
// Unityのようなレンダーキューには未対応で、全てのGameObjectとComponentを巡回して実行する。
void PlayerLoop::render()
{
    // ライトバッファの更新と転送
    LightManager::getInstance()->updateLightCBuffer();

    Camera* camera = Camera::main;
    if (camera != nullptr)
    {
        // カメラ単位の定数バッファ更新
        camera->UpdateConstantBuffer();

        // 不透明描画
        D3DManager::getInstance()->setCurrentCurrentRenderingMode(RenderingMode_Opaque);

        // 各コンポーネントの Render
        for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
        {
            render(&*it, *camera);
        }
    
        // 半透明描画
        D3DManager::getInstance()->setCurrentCurrentRenderingMode(RenderingMode_Transparent);

        // 各コンポーネントの Render
        for (auto& it : SceneManager::getInstance()->GetActiveScene()->GetRootGameObjects())
        {
            render(&*it, *camera);
        }
    }

    // UI
    for (auto& it : canvas_)
    {
        it->render();
    }
}


// 終了処理
void PlayerLoop::finalize()
{
    LightManager::destroy();
    Physics::destroy();
    SceneManager::destroy();
    D3DManager::destroy();
}


void PlayerLoop::awake(GameObject* object)
{
    // 自身のコンポーネントの中でAwakeを呼び出していないものを呼ぶ
    for (auto& it : object->GetComponents())
    {
        it->checkAwake();
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        awake(&*it);
    }
}


void PlayerLoop::fixedUpdate(GameObject* object)
{
    // アタッチされている各コンポーネントのFixedUpdateを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto behaviour = dynamic_cast<Behaviour*>(it.get());
        if (behaviour != nullptr && behaviour->enabled)
        {
            behaviour->FixedUpdate();
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        fixedUpdate(&*it);
    }
}


void PlayerLoop::checkStart(GameObject* object)
{
    // 自身のコンポーネントの中でStartを呼び出していないものを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto behaviour = dynamic_cast<Behaviour*>(it.get());
        if (behaviour != nullptr)
        {
            behaviour->checkStart();
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        checkStart(&*it);
    }
}


void PlayerLoop::update(GameObject* object)
{
    // アタッチされている各コンポーネントのUpdateを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto behaviour = dynamic_cast<Behaviour*>(it.get());
        if (behaviour != nullptr && behaviour->enabled)
        {
            behaviour->Update();
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        update(&*it);
    }
}


void PlayerLoop::lateUpdate(GameObject* object)
{
    // アタッチされている各コンポーネントのLateUpdateを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto behaviour = dynamic_cast<Behaviour*>(it.get());
        if (behaviour != nullptr && behaviour->enabled)
        {
            behaviour->LateUpdate();
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        lateUpdate(&*it);
    }
}


void PlayerLoop::render(GameObject* object, const Camera& camera)
{
    // アタッチされている各コンポーネントのRenderを呼ぶ
    for (auto& it : object->GetComponents())
    {
        auto renderer = dynamic_cast<Renderer*>(it.get());
        if (renderer != nullptr && renderer->enabled)
        {
            renderer->render(camera);
        }
    }

    // 子供のオブジェクトについて再帰
    for (auto& it : object->transform->getChildGameObjects())
    {
        render(&*it, camera);
    }
}


void PlayerLoop::registerCanvas(Canvas* c)
{
    canvas_.push_back(c);
}


void PlayerLoop::unregisterCanvas(Canvas* c)
{
    auto it = std::find(canvas_.begin(), canvas_.end(), c);
    if (it != canvas_.end()) canvas_.erase(it);
}


}