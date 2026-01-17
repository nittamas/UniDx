/**
 * @file Singleton.h
 * @brief シングルトンテンプレート。
 * 初期化と破棄のタイミングを制御したり、具体クラスで create できる
 */
#pragma once

#include <memory>
#include <assert.h>
#include "UniDxDefine.h"

namespace UniDx
{

/**
 * @brief シングルトンテンプレート。
 * 明示的な create と destroy が必要
 */
template<class T>
class Singleton
{
public:
    // インスタンスの取得
    static T* getInstance() { return instance_.get(); }

    // このクラスをインスタンスとして作成
    static void create()
    {
		assert(instance_ == nullptr);
        instance_ = std::make_unique<T>();
    }

    // インスタンスの破棄
	static void destroy()
	{
        instance_.reset();
	}

protected:
    Singleton() {}
    virtual ~Singleton() {}

    static unique_ptr<T> instance_;
};

template<class T>
inline std::unique_ptr<T> Singleton<T>::instance_ = nullptr;

}