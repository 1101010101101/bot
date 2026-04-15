#pragma once
#include <windows.h>

// ============================================================
//  Shared data structures — included by all translation units
// ============================================================

struct GameData {
    float localX, localY, localZ;
    float health;
    float yaw, pitch;

    struct Entity {
        float x, y, z;
        float health;
        char  name[64];
        bool  isPlayer;
    };

    Entity entities[256];
    int    entityCount;
};

extern GameData       g_data;
extern volatile bool  g_running;
