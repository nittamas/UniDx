#pragma once

#include <string_view>
#include <string>
#include <unordered_set>
#include <memory_resource>
#include <mutex>
#include <functional>

#include "UniDxDefine.h"
#include "Debug.h"

namespace UniDx
{

/**
 * @brief インターンプールに貯めて使う文字列
 * 高速に比較できる
 */
class StringId
{
public:
    static StringId intern(std::string_view sv);
    static StringId intern(const char8_t* s) { return intern(std::string_view(reinterpret_cast<const char*>(s))); }
    static StringId intern(std::u8string_view sv) { return intern(std::string_view(reinterpret_cast<const char*>(sv.data()), sv.size())); }

    StringId() {}
    constexpr std::u8string_view view() const noexcept { return view_; }
    const char* c_str() const { return reinterpret_cast<const char* >(view_.data()); } // NULL終端文字列あり

    friend bool operator==(StringId a, StringId b) noexcept {
        return a.view_.data() == b.view_.data(); // 同一性比較（内容比較しない）
    }
    friend bool operator!=(StringId a, StringId b) noexcept { return !(a == b); }

    explicit operator u8string() const { return u8string(view_.data(), view_.size()); }
    explicit operator std::string() const { return std::string(reinterpret_cast<const char*>(view_.data()), view_.size()); }
    explicit operator u8string_view() const { return u8string_view(view_.data(), view_.size()); }
    explicit operator std::string_view() const { return std::string_view(reinterpret_cast<const char*>(view_.data()), view_.size()); }

private:
    std::u8string_view view_;
    explicit StringId(const char8_t* v, size_t s) : view_(v, s) {} // InternPool側でNULL終端文字列を保証する

    friend class InternPool;
};
inline u8string ToString(StringId s) { return u8string(s); }


class InternPool
{
public:
    /** @brief インスタンスの取得*/
    static InternPool& instance()
    {
        static InternPool pool; // 静的初期化対応のため、UniDxの中では例外的に関数ローカルstaticにする
        return pool;
    }

    InternPool()
        : arena_(std::pmr::get_default_resource())
        , strings_(&arena_)
    {
    }

    StringId intern(std::string_view sv)
    {
        if(sv.empty()) return StringId();

        std::lock_guard lock(mtx_);

        auto it = strings_.find(sv); // 透明検索で string_view のまま探す
        if(it != strings_.end()) return StringId(reinterpret_cast<const char8_t*>(it->c_str()), it->size());

        // 新規に arena に確保して登録
        auto result = strings_.emplace(sv); // pmr::string(sv) が作られる
        return StringId(reinterpret_cast<const char8_t*>(result.first->c_str()), result.first->size());
    }

    void Log() const
    {
        for(auto& str : strings_)
        {
            Debug::Log(str.c_str());
        }
    }

private:
    // 透明検索用の Hash/Equal（std::string_view で find できるようにする）
    struct TransparentHash {
        using is_transparent = void;
        size_t operator()(std::string_view v) const noexcept { return std::hash<std::string_view>{}(v); }
        size_t operator()(const std::pmr::string& s) const noexcept { return std::hash<std::string_view>{}(std::string_view{ s }); }
    };
    struct TransparentEq {
        using is_transparent = void;
        bool operator()(std::string_view a, std::string_view b) const noexcept { return a == b; }
        bool operator()(const std::pmr::string& a, std::string_view b) const noexcept { return a == b; }
        bool operator()(std::string_view a, const std::pmr::string& b) const noexcept { return a == b; }
        bool operator()(const std::pmr::string& a, const std::pmr::string& b) const noexcept { return a == b; }
    };

private:
    std::mutex mtx_;
    std::pmr::monotonic_buffer_resource arena_;
    std::pmr::unordered_set<std::pmr::string, TransparentHash, TransparentEq> strings_;
};
inline StringId StringId::intern(std::string_view sv) { return InternPool::instance().intern(sv); }


}

namespace std
{
    template<>
    struct hash<UniDx::StringId>
    {
        size_t operator()(const UniDx::StringId& id) const noexcept { return std::hash<const void*>{}(id.view().data()); }
    };
}