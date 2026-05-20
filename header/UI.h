#pragma once

void begin2D();
void end2D();
void drawText(float x, float y, const char* text, void* font=0);
int textWidth(const char* text, void* font=0);
void drawTextCentered(float y, const char* text, void* font=0);
void drawPanel(float x1, float y1, float x2, float y2,
               float r, float g, float b, float a, bool border=true);
void drawMenuButton(float cx, float cy, float w, float h,
                    const char* label, bool selected, void* font=0);

void drawHUD();
void drawIntro();
void drawLogin();
void drawMenu();
void drawGameOver();
