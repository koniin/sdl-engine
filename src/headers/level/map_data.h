#ifndef MAP_DATA_H
#define MAP_DATA_H

#include "engine.h"

enum MapSize {
    Small = 0,
    Medium = 1,
    Large = 2,
    SIZE_OF_MapSize = 3
};
enum MapStyle {
    Desert = 0,
    SIZE_OF_MapStyle = 1
};

static const char* MapSizeNames[] = { "Small", "Medium", "Large" };
static const char* MapStyleNames[] = { "Desert" };

// statically check that the size of ColorNames fits the number of Colors
static_assert(sizeof(MapSizeNames)/sizeof(char*) == MapSize::SIZE_OF_MapSize, "MapSizeNames sizes dont match");
static_assert(sizeof(MapStyleNames)/sizeof(char*) == MapStyle::SIZE_OF_MapStyle, "MapStyleNames sizes dont match");

struct MapModifier {
    std::string name;
};

struct MapSettings {
    MapSize map_size;
    MapStyle style;
    std::vector<MapModifier> modifiers;
};

#endif