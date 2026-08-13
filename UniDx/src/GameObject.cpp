#include "pch.h"

#include <UniDx/Behaviour.h>


namespace UniDx{

// コピーコンストラクタ（Instantiate専用）
// Sceneへは接続せず、Awake/OnEnableも呼ばない
GameObject::GameObject(const GameObject& source) :
	Object([this]() { return name_; }),
	transform(nullptr),
	name_(source.name_),
	isCalledDestroy(false)
{
	components.reserve(source.components.size());

	// Componentを元と同じ順序でコピー構築
	for (const auto& sourceComponent : source.components)
	{
		auto clonedComponent = sourceComponent->copyConstruct();
		assert(clonedComponent != nullptr);

		clonedComponent->gameObject = this;
		Component* cloned = clonedComponent.get();
		components.push_back(std::move(clonedComponent));

		if (sourceComponent.get() == source.transform)
		{
			transform = dynamic_cast<Transform*>(cloned);
		}
	}

	assert(transform != nullptr);

	// Transformの所有構造を複製。未接続なのでAwakeは発生しない
	for (const auto& sourceChild : source.transform->getChildGameObjects())
	{
		auto clonedChild = std::unique_ptr<GameObject>(new GameObject(*sourceChild));
		Transform::SetParent(std::move(clonedChild), transform);
	}
}


// 階層内の全Componentがコピー構築可能か
bool GameObject::canInstantiate() const
{
	for (const auto& component : components)
	{
		if(!component->canCopyConstruct())
		{
			return false;
		}
	}

	for (const auto& child : transform->getChildGameObjects())
	{
		if (!child->canInstantiate()) return false;
	}

	return true;
}


// コピー構築済み階層へ、クローン固有の後処理を適用
void GameObject::cloneTo(GameObject& destination) const
{
	assert(components.size() == destination.components.size());
	for (size_t i = 0; i < components.size(); ++i)
	{
		components[i]->CloneTo(*destination.components[i]);
	}

	const auto& sourceChildren = transform->getChildGameObjects();
	const auto& destinationChildren = destination.transform->getChildGameObjects();
	assert(sourceChildren.size() == destinationChildren.size());

	for (size_t i = 0; i < sourceChildren.size(); ++i)
	{
		sourceChildren[i]->cloneTo(*destinationChildren[i]);
	}
}


// GameObjectの複製
GameObject* Instantiate(const GameObject& original, Transform* parent)
{
	if (parent == nullptr || !original.canInstantiate()) return nullptr;

	auto clone = std::unique_ptr<GameObject>(new GameObject(original));
	GameObject* result = clone.get();

	// 全階層の構築完了後、Sceneへ接続する前に後処理
	original.cloneTo(*result);

	// 接続後、既存の接続判定によってAwake/OnEnableが同期実行される
	Transform::SetParent(std::move(clone), parent);
	return result;
}


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
