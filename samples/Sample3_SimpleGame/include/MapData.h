#pragma once

#include <memory>
#include <string>
#include <vector>

class MapData
{
public:
	static void create() { _instance = std::make_unique<MapData>(); }
	static MapData* getInstance() { return _instance.get(); }

	char getData(int x, int y) const;
	size_t getHeight() const;
	size_t getWidth() const;
	bool load(std::u8string_view filename);

protected:
	static inline std::unique_ptr<MapData> _instance;

	std::vector< std::string > data;
};

