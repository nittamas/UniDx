#include "pch.h"

#include <UniDx/Font.h>

#include <filesystem>
#include <SpriteFont.h>
#include <UniDx/D3DManager.h>


namespace UniDx
{

using namespace DirectX;


Font::Font() : Object([this]() { return fileName; })
{
}

bool Font::Load(std::wstring filePath)
{
	spriteFont = std::make_unique<DirectX::SpriteFont>(D3DManager::getInstance()->GetDevice().Get(), filePath.c_str());
	std::filesystem::path path(filePath);
	fileName = StringId::intern(path.filename().u8string());
	return spriteFont != nullptr;
}

SpriteFont* Font::getSpriteFont() const
{
	return spriteFont.get();
}

}