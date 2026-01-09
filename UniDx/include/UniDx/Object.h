#pragma once
#include <string>

#include "UniDxDefine.h"
#include "StringId.h"
#include "Property.h"

namespace UniDx {

// --------------------
// Object基底クラス
// --------------------
class Object
{
public:
    virtual ~Object() {}

    ReadOnlyProperty<StringId> name;

    Object(ReadOnlyProperty<StringId>::Getter nameGet) : name(nameGet) {}
};

} // namespace UniDx
