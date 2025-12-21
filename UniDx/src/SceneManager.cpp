#include "pch.h"
#include <UniDx/SceneManager.h>

#include <memory>

#include <UniDx/Material.h>


namespace UniDx{

using namespace std;


// シーン作成
void SceneManager::createScene()
{
	activeScene = std::move(CreateDefaultScene());
//	defaultMaterial = make_unique<Material>();
//	defaultMaterial->shader.compile<VertexPN>(L"Resource/DefaultShade.hlsl");
}

// デストラクタ。シーンを破棄
SceneManager::~SceneManager()
{
	DestroyDefaultScene();
}

}
