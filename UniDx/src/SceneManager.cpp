#include "pch.h"
#include <UniDx/SceneManager.h>

#include <memory>

#include <UniDx/GameObject.h>
#include <UniDx/Material.h>


namespace UniDx{

using namespace std;


// GameObjectがアクティブシーンのツリーに接続されているか
bool IsConnectedToActiveScene(const GameObject* gameObject)
{
	SceneManager* sceneManager = SceneManager::getInstance();
	if (sceneManager == nullptr || gameObject == nullptr) return false;

	Scene* activeScene = sceneManager->GetActiveScene();
	if (activeScene == nullptr) return false;

	// 親をたどって、このGameObjectが属するツリーのルートを取得
	const GameObject* root = gameObject;
	while (root->transform->parent != nullptr)
	{
		root = root->transform->parent->gameObject;
	}

	// そのルートがアクティブシーンに所有されているか確認
	for (const auto& sceneRoot : activeScene->GetRootGameObjects())
	{
		if (sceneRoot.get() == root) return true;
	}

	return false;
}


// シーン作成
void SceneManager::createScene()
{
	activeScene = std::move(CreateDefaultScene());
}


// デストラクタ。シーンを破棄
SceneManager::~SceneManager()
{
	DestroyDefaultScene();
}

}
