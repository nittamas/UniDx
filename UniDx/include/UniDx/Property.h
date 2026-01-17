/**
 * @file Property.h
 *
 * @brief C#のプロパティライクな記述を実現するクラス
 * ReadOnlyProperty<> 読み取り専用プロパティ
 * Property<> 読み書きプロパティ
 * メンバアクセス(.演算)が使えないなどの制約がある
 */
#pragma once

#include <functional>
#include "UniDxDefine.h"

namespace UniDx
{

/// @brief 読み取り専用プロパティ
template<typename T>
class ReadOnlyProperty
{
public:
    using Getter = std::function<T()>;

    /** @brief Getterを与えるコンストラクタ*/
    ReadOnlyProperty(Getter getter)
        : getter_(getter) {
    }

    /** @brief 値の取得*/
    T get() const { return getter_(); }

    /** @brief 値の変換*/
    operator T() const { return getter_(); }

    /** @brief 三方比較演算*/
    template<typename U>
    auto operator<=>(const U& rhs) const { return getter_() <=> rhs; }

protected:
    Getter getter_;
};

/// @brief 読み取り専用プロパティポインタ版
template<typename T>
class ReadOnlyProperty<T*>
{
public:
    using Getter = std::function< T* ()>;

    ReadOnlyProperty(Getter getter)
        : getter_(getter) {
    }

    /** @brief 値の取得*/
    T* get() const { return getter_(); }

    /** @brief 値の変換*/
    operator T*() const { return getter_(); }

    /** @brief メンバアクセス*/
    T* operator->() { return getter_(); }
    const T* operator->() const { return getter_(); }

    // ポインタ比較演算
    template<typename U>
    bool operator==(U* rhs) const { return getter_() == rhs; }
    template<typename U>
    bool operator!=(U* rhs) const { return getter_() != rhs; }

protected:
    Getter getter_;
};


/// @brief 読み書きプロパティ
template<typename T>
class Property : public ReadOnlyProperty<T>
{
public:
    using Getter = ReadOnlyProperty<T>::Getter;
    using Setter = std::function<void(const T&)>;

    Property(Getter getter, Setter setter)
        : ReadOnlyProperty<T>(getter), setter_(setter) {
    }

    /** @brief 値の設定*/
    void set(const T& value) { setter_(value); }

    /** @brief C#風代入アクセス*/
    template<typename U>
    Property& operator=(const U& value) { set(T(value)); return *this; }

private:
    Setter setter_;
};

template<typename T>
inline u8string ToString(const ReadOnlyProperty<T>& v) { return ToString(v.get()); }
template<typename T>
inline u8string ToString(const Property<T>& v) { return ToString(v.get()); }


}
