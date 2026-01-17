// CreateDefaultScene.cpp
// デフォルトのシーンを生成します

#include <numbers>

#include <UniDx.h>
#include <UniDx/Scene.h>
#include <UniDx/PrimitiveRenderer.h>
#include <UniDx/GltfModel.h>
#include <UniDx/Canvas.h>
#include <UniDx/TextMesh.h>
#include <UniDx/Font.h>
#include <UniDx/LightManager.h>

#include "ModelController.h"
#include "LightController.h"

using namespace std;
using namespace UniDx;


unique_ptr<Scene> CreateDefaultScene()
{
    // -- キャラクター --
    auto character = make_unique<GameObject>(u8"キャラクター", Vector3(0, -0.8f, 0),
        make_unique<GltfModel>()
    );
    auto model = character->GetComponent<GltfModel>(true);

    // .glbファイルからモデルデータをロード
    // 先にGameObjectにアタッチしておく
    model->Load<VertexSkin>(
        u8"resource/mini_emma.glb",
        u8"resource/SkinBasic.hlsl");
    character->transform->localRotation = Quaternion::Euler(0, 180, 0);

    // -- ライト --
    auto light = make_unique<GameObject>(u8"ディレクショナルライト",

        // ライトの作成。デフォルトは LightType_Directional
        make_unique<Light>(),

        // ライト操作プログラム
        make_unique<LightController>()
    );
    light->transform->localPosition = Vector3(-1.0f, 3.0f, -4.0f);
    light->GetComponent<Light>(true)->intensity = 0.6f;

    // 環境光
    LightManager::getInstance()->ambientColor = Color(0.5f, 0.5f, 0.5f);

    // -- UI --
    // フォントを作成
    auto font = make_shared<Font>();
    font->Load(u8"resource/M PLUS 1.spritefont"); // 英数字とひらがな・カタカナのみスプライト化

    // テキストメッシュコンポーネントを作成
    auto textMesh = make_unique<TextMesh>();
    textMesh->font = font;
    textMesh->text = u8"ミニエマ\n WASD:かいてん\n OP:ライト";

    auto textObj = make_unique<GameObject>(u8"テキスト", textMesh);
    textObj->transform->localPosition = Vector3(100, 20, 0);
    textObj->transform->localScale = Vector3(0.6f, 0.6f, 1.0f);

    auto canvas = make_unique<Canvas>();
    canvas->LoadDefaultMaterial(u8"resource");

    // シーンを作って戻す
    return make_unique<Scene>(

        make_unique<GameObject>(u8"キャラルート",
            move(character),
            make_unique<ModelController>()
        ),

        move(light),

        make_unique<GameObject>(u8"カメラ", Vector3(0, 0, -2.5),
            make_unique<Camera>()
        ),

        make_unique<GameObject>(u8"UI",
            move(canvas),
            move(textObj)
        )
    );
}


void DestroyDefaultScene()
{
}