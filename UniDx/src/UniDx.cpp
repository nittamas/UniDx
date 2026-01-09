#include "pch.h"

#include <Windows.h>


namespace UniDx
{

u8string ToUtf8(std::wstring_view wstr)
{
    if (wstr.empty()) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    u8string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), reinterpret_cast<char*>(&strTo[0]), size_needed, nullptr, nullptr);
    return strTo;
}

std::wstring ToUtf16(u8string_view str)
{
    if (str.empty()) return {};
    auto cp = reinterpret_cast<const char*>(str.data());
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, cp, (int)str.size(), nullptr, 0);
    std::wstring strTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, cp, (int)str.size(), &strTo[0], size_needed);
    return strTo;
}

}
