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
#include <UniDx/Image.h>
#include <UniDx/LightManager.h>

#include "CameraController.h"
#include "Player.h"
#include "MapData.h"
#include "LightController.h"

#include <thread>
#include "MainGame.h"


using namespace std;
using namespace UniDx;


void MainGame::createMap()
{
    // マップデータ作成
    MapData::create();
    MapData::getInstance()->load(u8"resource/map_data.txt");

    // マテリアルの作成
    auto wallMat = std::make_shared<Material>();
    auto floorMat = std::make_shared<Material>();
    auto coinMat = std::make_shared<Material>();

    // シェーダを指定してコンパイル
    wallMat->shader->compile<VertexPNT>(u8"resource/AlbedoShadeSpec.hlsl");
    floorMat->shader->compile<VertexPNT>(u8"resource/AlbedoShadeSpec.hlsl");
    floorMat->color = Color(0.85f, 0.8f, 0.85f);
    coinMat->shader->compile<VertexPN>(u8"resource/ShadeSpec.hlsl");
    coinMat->color = Color(1.0f, 0.9f, 0.1f);

    // 床テクスチャ作成
    auto floorTex = std::make_shared<Texture>();
    floorTex->Load(u8"resource/floor.png");
    floorMat->AddTexture(std::move(floorTex));

    // 壁テクスチャ作成
    auto wallTex = std::make_shared<Texture>();
    wallTex->Load(u8"resource/wall.png");
    wallMat->AddTexture(std::move(wallTex));

    // マップ作成
    auto map = make_unique<GameObject>();

    // 各ブロック作成
    for (int i = 0; i < MapData::getInstance()->getWidth(); i++)
    {
        for (int j = 0; j < MapData::getInstance()->getHeight(); j++)
        {
            switch (MapData::getInstance()->getData(i, j))
            {
            case '#':
            {
                auto rb = make_unique<Rigidbody>();
                rb->gravityScale = 0;
                rb->mass = numeric_limits<float>::infinity();

                // 壁オブジェクトを作成
                auto wall = make_unique<GameObject>(u8"壁",
                    CubeRenderer::create<VertexPNT>(wallMat),
                    move(rb),
                    make_unique<AABBCollider>());
                wall->transform->localScale = Vector3(2, 2, 2);
                wall->transform->localPosition = Vector3(
                    i * 2 - float(MapData::getInstance()->getWidth() / 2) * 2,
                    0,
                    j * -2 + float(MapData::getInstance()->getHeight() / 2) * 2
                );

                // 壁の親をマップにする
                Transform::SetParent(move(wall), map->transform);
            }
            break;

            case 'C':
            {
                // コインオブジェクトを作成
                auto coin = make_unique<GameObject>(u8"Coin",
                    make_unique<GltfModel>(),
                    make_unique<Rigidbody>(),
                    make_unique<SphereCollider>(Vector3(0, -0.1f, 0), 0.4f)
                );
                auto model = coin->GetComponent<GltfModel>(true);
                model->Load<VertexPN>(
                    u8"resource/coin.glb",
                    coinMat);

                coin->transform->localPosition = Vector3(
                    i * 2 - float(MapData::getInstance()->getWidth() / 2) * 2,
                    -0.5f,
                    j * -2 + float(MapData::getInstance()->getHeight() / 2) * 2
                );
                coin->transform->localScale = Vector3(3, 3, 3);

                // コインの親をマップにする
                Transform::SetParent(move(coin), map->transform);
            }
            break;

            default:
                break;
            }

            // 床
            if (i % 2 == 0 && j % 2 == 0)
            {
                auto rb = make_unique<Rigidbody>();
                rb->gravityScale = 0;
                rb->mass = numeric_limits<float>::infinity();
                auto floor = make_unique<GameObject>(u8"床",
                    CubeRenderer::create<VertexPNT>(floorMat),
                    move(rb),
                    make_unique<AABBCollider>());
                floor->transform->localScale = Vector3(4, 1, 4);
                floor->transform->localPosition = Vector3(
                    i * 2 - float(MapData::getInstance()->getWidth() / 2) * 2 + 1.0f,
                    -1.5f,
                    j * -2 + float(MapData::getInstance()->getHeight() / 2) * 2 - 1.0f
                );

                // 壁の親をマップにする
                Transform::SetParent(move(floor), map->transform);
            }
        }
    }

    mapObj = move(map);
}


unique_ptr<UniDx::Scene> MainGame::CreateScene()
{
    // -- プレイヤー --
    auto playerObj = make_unique<GameObject>(u8"プレイヤー",
        make_unique<GltfModel>(),
        make_unique<Rigidbody>(),
        make_unique<SphereCollider>(Vector3(0, 0.25f, 0)),
        make_unique<Player>()
        );
    auto model = playerObj->GetComponent<GltfModel>(true);
    model->Load<VertexSkin>(
        u8"resource/mini_emma.glb",
        u8"resource/SkinBasic.hlsl");
    playerObj->transform->localPosition = Vector3(0, -1, 0);
    playerObj->transform->localRotation = Quaternion::Euler(0, 180, 0);

    // -- カメラ --
    auto cameraBehaviour = make_unique<CameraController>();
    cameraBehaviour->player = playerObj->GetComponent<Player>(true);

    // -- ライト --
    LightManager::getInstance()->ambientColor = Color(0.3f, 0.3f, 0.3f, 1.0f);

    auto lights = make_unique<GameObject>(u8"ライト群");
    auto light = make_unique<GameObject>(u8"ディレクショナルライト", make_unique<Light>(), make_unique<LightController>());
    light->transform->localPosition = Vector3(4, 3, 0);
    light->GetComponent<Light>(true)->intensity = 0.4f;
    Transform::SetParent(move(light), lights->transform);

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            auto l = make_unique<Light>();
            l->type = LightType_Point;
            l->range = 10.0f;

            auto light = make_unique<GameObject>(u8"ポイントライト",
                move(l),
                make_unique<LightController>());
            light->transform->localPosition = Vector3(10.0f * j - 5.0f, 4,  10.0f * i - 5.0f);
            Transform::SetParent(move(light), lights->transform);
        }
    }

    // -- UI --
    auto font = make_shared<Font>();
    font->Load(u8"resource/M PLUS 1.spritefont");
    auto textMesh = make_unique<TextMesh>();
    textMesh->font = font;
    textMesh->text = u8"WASD:いどう\nIJKL:カメラ\nOP:ライト";

    auto textObj = make_unique<GameObject>(u8"テキスト", textMesh);
    textObj->transform->localPosition = Vector3(100, 20, 0);
    textObj->transform->localScale = Vector3(0.6f, 0.6f, 1.0f);

    auto scoreMesh = make_unique<TextMesh>();
    scoreMesh->font = font;
    scoreMesh->text = u8"0";
    scoreTextMesh = scoreMesh.get();

    auto scoreTextObj = make_unique<GameObject>(u8"スコア", scoreMesh);
    scoreTextObj->transform->localPosition = Vector3(480, 20, 0);

    auto canvas = make_unique<Canvas>();
    canvas->LoadDefaultMaterial(u8"resource");

    // -- マップデータ --
    createMap();

    // シーンを作って戻す
    return make_unique<Scene>(

        make_unique<GameObject>(u8"オブジェクトルート",
            move(playerObj),
            move(mapObj)
        ),

        move(lights),

        make_unique<GameObject>(u8"カメラルート", Vector3(0, 3, -5),
            make_unique<Camera>(),
            move(cameraBehaviour)
        ),

        make_unique<GameObject>(u8"UI",
            move(canvas),
            move(textObj),
            move(scoreTextObj)
        )
    );
}


MainGame::~MainGame()
{
}


void MainGame::AddScore(int n)
{
    score += n;
    scoreTextMesh->text = ToString(score);
}
