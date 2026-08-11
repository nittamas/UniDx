#include "pch.h"

#include <UniDx/Behaviour.h>


namespace UniDx{

// デストラクタ
// コンポーネントのデストラクタより前にdoDestroy()を呼んでおく
GameObject::~GameObject()
{
	for (auto& i : components)
	{
		i->doDestroy(); // 破棄処理
	}
}


// 自身と子孫について、未呼び出しのAwake()/OnEnable()を呼ぶ
// checkAwake()は冪等なので何度呼んでも安全
void GameObject::checkAwake()
{
	// 自身のコンポーネントの中でAwakeを呼び出していないものを呼ぶ
	// Awake()中の追加に備えてインデックスで巡回する
	for (size_t i = 0; i < components.size(); ++i)
	{
		components[i]->checkAwake();
	}

	// 子供のオブジェクトについて再帰
	auto& children = transform->getChildGameObjects();
	for (size_t i = 0; i < children.size(); ++i)
	{
		children[i]->checkAwake();
	}
}


// Destroy()が呼ばれたコンポーネントを削除
// 自身を削除する場合 true
bool GameObject::checkDestroy()
{
	// 自身の削除チェック
	if (isCalledDestroy && transform->parent != nullptr)
	{
		transform->SetParent(nullptr);
		return true;
	}

	// 子供のオブジェクトについて再帰
	for(int i = 0; i < transform->getChildGameObjects().size();)
	{
		auto& o = transform->getChildGameObjects()[i];
		if (!o->checkDestroy())
		{
			++i; // 削除しないとき次
		}
	}

	// Destroyが呼ばれたコンポーネントを削除
	for (auto it = components.begin(); it != components.end();)
	{
		if ((*it) != nullptr && (*it)->isDestroyed())
		{
			(*it)->doDestroy(); // 破棄処理
			it = components.erase(it); // コンポーネントを削除
		}
		else
		{
			++it;
		}
	}
	return false;
}


void GameObject::onTriggerEnter(Collider* other)
{
	for (auto& i : components)
	{
		Behaviour* b = dynamic_cast<Behaviour*>(i.get());
		if(b != nullptr) b->OnTriggerEnter(other);
	}
}


void GameObject::onTriggerStay(Collider* other)
{
	for (auto& i : components)
	{
		Behaviour* b = dynamic_cast<Behaviour*>(i.get());
		if(b != nullptr) b->OnTriggerStay(other);
	}
}


void GameObject::onTriggerExit(Collider* other)
{
	for (auto& i : components)
	{
		Behaviour* b = dynamic_cast<Behaviour*>(i.get());
		if(b != nullptr) b->OnTriggerExit(other);
	}
}


void GameObject::onCollisionEnter(const Collision& collision)
{
	for (auto& i : components)
	{
		Behaviour* b = dynamic_cast<Behaviour*>(i.get());
		if(b != nullptr) b->OnCollisionEnter(collision);
	}
}


void GameObject::onCollisionStay(const Collision& collision)
{
	for (auto& i : components)
	{
		Behaviour* b = dynamic_cast<Behaviour*>(i.get());
		if(b != nullptr) b->OnCollisionStay(collision);
	}
}


void GameObject::onCollisionExit(const Collision& collision)
{
	for (auto& i : components)
	{
		Behaviour* b = dynamic_cast<Behaviour*>(i.get());
		if(b != nullptr) b->OnCollisionExit(collision);
	}
}

void Destroy(GameObject* gameObject)
{
	assert(gameObject != nullptr);
	gameObject->isCalledDestroy = true; // フレームの終わりに削除される
}

}
