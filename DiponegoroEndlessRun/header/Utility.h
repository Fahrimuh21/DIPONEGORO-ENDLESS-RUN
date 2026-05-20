#pragma once

float laneX(int l);
float lerp(float a, float b, float t);
float clampf(float v, float lo, float hi);
void hsvToRgb(float h, float s, float v, float &r, float &g, float &b);

void drawCylinder(float radius, float height, int slices = 10);
void drawSphere(float radius, int slices = 10, int stacks = 8);
