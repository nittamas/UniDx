#pragma once

#include <memory>

#include "UniDxDefine.h"
#include "Singleton.h"

namespace UniDx
{

class GameObject;

// シーン
class Scene
{
public:
    typedef std::vector<unique_ptr<GameObject>> GameObjectContainer;

    Scene() {}

    // 可変長引数でunique_ptr<GameObject>を受け取るコンストラクタ
    template<typename... GameObjectPtrs>
    Scene(GameObjectPtrs&&... objs) {
        AddGameObjects(std::forward<GameObjectPtrs>(objs)...);
    }

    const GameObjectContainer& GetRootGameObjects() { return routeGameObjects; }

protected:
    GameObjectContainer routeGameObjects;

    // ヘルパー関数でパック展開
    void AddGameObjects() {}

    template<typename First, typename... Rest>
    void AddGameObjects(First&& first, Rest&&... rest)
    {
        routeGameObjects.push_back(std::move(first));
        AddGameObjects(std::forward<Rest>(rest)...);
    }
};

}