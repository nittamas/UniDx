#pragma once
#include <string>

#include "UniDxDefine.h"
#include "Property.h"

namespace UniDx {

// --------------------
// Object基底クラス
// --------------------
class Object
{
public:
    virtual ~Object() {}

    ReadOnlyProperty<wstring_view> name;

    Object(ReadOnlyProperty<wstring_view>::Getter nameGet) : name(nameGet) {}
};

} // namespace UniDx
