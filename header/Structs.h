#pragma once

enum GameState  { INTRO, LOGIN, MENU, PLAYING, PAUSED, GAMEOVER };
enum CameraMode { CAM_BACK, CAM_SIDE, CAM_FIRST };

struct Obstacle {
    float z;
    int lane;
    int type;
    bool active;
    float length;
    int color;
};

struct Coin {
    float x, y, z;
    bool active;
    float rot;
};

struct Building {
    float z, h, w;
    int side;
    float r, g, b;
};

struct Lamp {
    float z;
    int side;
};

struct Cloud {
    float x, y, z, speed, scale;
};