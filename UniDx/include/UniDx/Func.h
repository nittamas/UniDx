#pragma once

namespace UniDx
{

/**
 * @brief メンバー関数オブジェクト
 * コンパイル時に型を指定するので高速
 */
template<class C, class R, class... Args>
struct MemberFunc
{
    using MemFn       = R (C::*)(Args...);

    C*    object = nullptr;
    MemFn func   = nullptr;

    bool isValid() const noexcept { return object && func; }

    R operator()(Args... args) const
    {
        return (object->*func)(std::forward<Args>(args)...);
    }
};
template<class C, class R, class... Args>
MemberFunc<C, R, Args...> MakeMemberFunc(C* obj, R (C::* func)(Args...)) { return { obj, func }; }


/**
 * @brief 戻り値のないメンバー関数オブジェクト
 * コンパイル時に型を指定するので高速
 */
template<class C, class... Args>
struct MemberAction
{
    using MemFn = void(C::*)(Args...);

    C* object = nullptr;
    MemFn func = nullptr;

    bool isValid() const noexcept { return object && func; }

    void operator()(Args... args) const
    {
        (object->*func)(std::forward<Args>(args)...);
    }
};
template<class C, class... Args>
MemberAction<C, Args...> MakeMemberAction(C* obj, void(C::* func)(Args...)) { return { obj, func }; }


}
