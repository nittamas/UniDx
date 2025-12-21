#pragma once

#include <memory>

#include "Singleton.h"
#include "Scene.h"
#include "Material.h"


std::unique_ptr<UniDx::Scene> CreateDefaultScene();
void DestroyDefaultScene();


namespace UniDx
{

class Scene;

// シーンマネージャ
class SceneManager : public Singleton<SceneManager>
{
public:
    ~SceneManager();

    void createScene();

    Scene* GetActiveScene() { return activeScene.get(); }

protected:
    std::unique_ptr<Scene> activeScene;
};

}