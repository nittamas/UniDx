#pragma once

#include <functional>

//
// C#のプロパティライクな記述を実現するクラス
// ReadOnlyProperty<>
// Property<>
// 
namespace UniDx
{

// 読み取り専用プロパティ
template<typename T>
class ReadOnlyProperty
{
public:
    using Getter = std::function<T()>;

    // Getterを与えるコンストラクタ
    ReadOnlyProperty(Getter getter)
        : getter_(getter) {
    }

    // 値の取得
    T get() const { return getter_(); }

    // 値の変換
    operator T() const { return getter_(); }

    // 三方比較演算
    template<typename U>
    auto operator<=>(const U& rhs) const { return getter_() <=> rhs; }

protected:
    Getter getter_;
};

// 読み取り専用プロパティポインタ版
template<typename T>
class ReadOnlyProperty<T*>
{
public:
    using Getter = std::function< T* ()>;

    ReadOnlyProperty(Getter getter)
        : getter_(getter) {
    }

    // 値の取得
    T* get() const { return getter_(); }

    // 値の変換
    operator T*() const { return getter_(); }

    // メンバアクセス
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


// 読み書きプロパティ
template<typename T>
class Property : public ReadOnlyProperty<T>
{
public:
    using Getter = ReadOnlyProperty<T>::Getter;
    using Setter = std::function<void(const T&)>;

    Property(Getter getter, Setter setter)
        : ReadOnlyProperty<T>(getter), setter_(setter) {
    }

    // 値の設定
    void set(const T& value) { setter_(value); }

    // C#風のアクセス
    Property& operator=(const T& value) { set(value); return *this; }

private:
    Setter setter_;
};

}
