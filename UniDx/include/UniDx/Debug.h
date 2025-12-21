#pragma once

#include <string>
#include <DirectXMath.h>

#ifdef _DEBUG

namespace UniDx
{

// デバッグ用ネームスペース
namespace Debug
{
    inline void log_(const wchar_t* value)
    {
        OutputDebugStringW(value);
        OutputDebugStringW(L"\n");
    }
    inline void log_(const char* value)
    {
        OutputDebugStringA(value);
        OutputDebugStringA("\n");
    }

    template<typename T>
    inline void Log(const T& v) { log_(ToString(v).c_str()); }
    inline void Log(const wchar_t* value) { log_(value); }
    inline void Log(const std::string& value) { log_(value.c_str()); }
    inline void Log(const std::string_view& value) { log_(std::string(value).c_str()); }
    inline void Log(const char* value) { log_(value); }
}

}

#else

namespace UniDx
{

namespace Debug
{
    template<typename T>
    inline void Log(const T& value) { }
}

}

#endif
