#pragma once

#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

namespace UniDx
{

using std::u8string;
using std::u8string_view;
using std::unique_ptr;
using std::shared_ptr;
using std::make_unique;
using std::make_shared;
using Microsoft::WRL::ComPtr;

class Object;
class GameObject;
class Component;
class Transform;

// レンダリングモード
enum RenderingMode
{
	RenderingMode_Opaque,
	RenderingMode_Transparent
};

/** @brief utf8文字列へ変換*/
u8string ToUtf8(std::wstring_view wstr);

/** @brief utf16文字列へ変換*/
std::wstring ToUtf16(std::u8string_view str);

/** @brief stringへ変換*/
inline std::string str(std::u8string_view u8) { return { reinterpret_cast<const char*>(u8.data()), u8.size() }; }

template<typename T>
inline u8string ToString(const T& v) { return u8string( reinterpret_cast<const char8_t*>(std::to_string(v).c_str())); }
inline u8string ToString(const char8_t* v) { return u8string(v); }
inline u8string ToString(const std::wstring& v) { return ToUtf8(v); }
inline u8string ToString(const std::wstring_view& v) { return ToUtf8(v); }
inline u8string ToString(const std::u8string& v) { return v; }
inline u8string ToString(const std::u8string_view& v) { return std::u8string(v).c_str(); }
inline u8string ToString(const char* v) { return u8string(reinterpret_cast<const char8_t*>(v)); }
inline u8string ToString(const std::string& v) { return u8string(reinterpret_cast<const char8_t*>(v.data()), v.size()); }
inline u8string ToString(const DirectX::XMFLOAT2& v)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3) << "(" << v.x << ", " << v.y << ")";
    return u8string(reinterpret_cast<const char8_t*>(ss.str().c_str()));
}
inline u8string ToString(const DirectX::XMFLOAT3& v)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3) << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return u8string(reinterpret_cast<const char8_t*>(ss.str().c_str()));
}
inline u8string ToString(const DirectX::XMFLOAT4& v)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3) << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return u8string(reinterpret_cast<const char8_t*>(ss.str().c_str()));
}

}
