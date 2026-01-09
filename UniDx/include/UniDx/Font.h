#pragma once

#include "Object.h"

/// \cond DOXYGEN_IGNORE
namespace DirectX
{
	inline namespace DX11
	{
		class SpriteFont;
	}
}
/// \endcond

namespace UniDx {

class Font : public Object
{
public:
	Font();

	bool Load(u8string filePath) { return Load(ToUtf16(filePath)); }
	bool Load(std::wstring filePath);

	DirectX::SpriteFont* getSpriteFont() const;

private:
	StringId fileName;
	unique_ptr<DirectX::SpriteFont> spriteFont;

	
};

}