#include "MapData.h"

#include <fstream>


char MapData::getData(int x, int y) const
{
	return data[y][x];
}

size_t MapData::getHeight() const
{
	return data.size();
}

size_t MapData::getWidth() const
{
	return data[0].size();
}

bool MapData::load(std::u8string_view filename)
{
    std::ifstream file(reinterpret_cast<const char*>(filename.data()));
    if (!file) {
        return false; // ファイルが開けなかった
    }

    data.clear();

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty())
        {
            data.push_back(line);
            // TODO:lineから情報を読み取って保存する
        }
    }
    return true;
}
