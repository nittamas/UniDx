#include "pch.h"

#include <UniDx/Behaviour.h>


namespace UniDx{


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


}
