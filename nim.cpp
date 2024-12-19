#include <fstream>
#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

int main() {
    int screenWidth = 800;
    int screenHeight = 600;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "nim");
    SetExitKey(KEY_NULL);
    
    std::vector<std::string> text;

    std::ifstream file("./nim.cpp");
    std::string line = "";
    while (std::getline(file, line)) {
        text.push_back(line);
    }

    int fontSize = 22;
    int cursorX = 0;
    int cursorY = 0;
    float blinkTime = 0.0f;
    bool showCursor = true;

    bool isFullscreen = false;

    float backspaceTime = 0.0f;
    bool backspaceHeld = false;
    float arrowTime = 0.0f;
    bool arrowHeld = false;
    float keyHeldTime = 0.3f;

    Font codeFont = LoadFontEx("font.ttf", fontSize, 0, 0);
    int lineHeight = fontSize + 5;

    int scrollOffset = 0;
    int maxVisibleLines = screenHeight / lineHeight;

    SetTargetFPS(40);

    while (!WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        maxVisibleLines = screenHeight / lineHeight;

        // Blink cursor
        blinkTime += GetFrameTime();
        if (blinkTime >= 0.5f) {
            showCursor = !showCursor;
            blinkTime = 0.0f;
        }

        // Handle scrolling
        scrollOffset -= GetMouseWheelMove() * 3;
        scrollOffset = std::clamp(scrollOffset, 0, std::max(0, (int)text.size() - maxVisibleLines));

        // Update input logic for backspace and arrow keys (unchanged)
        if (IsKeyDown(KEY_BACKSPACE)) {
            backspaceTime += GetFrameTime();
            showCursor = true;
            if (backspaceTime > keyHeldTime || !backspaceHeld) {
                if (cursorX > 0) {
                    text[cursorY].erase(cursorX - 1, 1);
                    cursorX--;
                } else if (cursorY > 0) {
                    cursorX = text[cursorY - 1].size();
                    text[cursorY - 1] += text[cursorY];
                    text.erase(text.begin() + cursorY);
                    cursorY--;
                }
                backspaceHeld = true;
            }
        } else {
            backspaceTime = 0.0f;
            backspaceHeld = false;
        }

        // Arrow key handling (unchanged)
		if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN)) {
			showCursor = true;
            arrowTime += GetFrameTime();
            if (arrowTime > keyHeldTime || !arrowHeld) {
                if (IsKeyDown(KEY_LEFT)) {
                    if (cursorX > 0) cursorX--;
                    else if (cursorY > 0) {
                        cursorY--;
                        cursorX = text[cursorY].size();
                    }
                } else if (IsKeyDown(KEY_RIGHT)) {
                    if (cursorX < text[cursorY].size()) cursorX++;
                    else if (cursorY < text.size() - 1) {
                        cursorY++;
                        cursorX = 0;
                    }
                } else if (IsKeyDown(KEY_UP)) {
                    if (cursorY > 0) {
                        cursorY--;
                        cursorX = (cursorX > text[cursorY].size()) ? text[cursorY].size() : cursorX;
                    }
                } else if (IsKeyDown(KEY_DOWN)) {
                    if (cursorY < text.size() - 1) {
                        cursorY++;
                        cursorX = (cursorX > text[cursorY].size()) ? text[cursorY].size() : cursorX;
                    }
                }
                arrowHeld = true;
            }
        } else {
            arrowTime = 0.0f;
            arrowHeld = false;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            showCursor = true;
            std::string remaining = text[cursorY].substr(cursorX);
            text[cursorY] = text[cursorY].substr(0, cursorX);
            text.insert(text.begin() + cursorY + 1, remaining);
            cursorX = 0;
            cursorY++;
        }

        if (IsKeyPressed(KEY_TAB)) {
            showCursor = true;
            text[cursorY].insert(cursorX, "    ");
            cursorX += 4;
        }

        if (IsKeyPressed(KEY_F11)) {
            showCursor = true;
            isFullscreen = !isFullscreen;
            if (isFullscreen) {
                ToggleFullscreen();
            } else {
                SetWindowSize(screenWidth, screenHeight);
                SetWindowPosition(100, 100);
            }
        }

        if (IsKeyPressed(KEY_F10)) {
            showCursor = true;
            MinimizeWindow();
        }

		int key = GetCharPressed();
        while (key > 0) {
            showCursor = true;
            if (key >= 32 && key <= 126) {
                showCursor = true;
                text[cursorY].insert(cursorX, 1, (char)key);
                cursorX++;

                float textWidth = MeasureTextEx(codeFont, text[cursorY].c_str(), fontSize, 1).x;

                if (textWidth > screenWidth - 50) {
                    std::string remaining = text[cursorY].substr(cursorX);
                    text[cursorY] = text[cursorY].substr(0, cursorX);
                    text.insert(text.begin() + cursorY + 1, remaining);
                    cursorY++;
                    cursorX = 0;
                }
            }
            key = GetCharPressed();
        }
        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawRectangle(0, 0, screenWidth, screenHeight, DARKGRAY);

        int startLine = scrollOffset;
        int endLine = std::min(startLine + maxVisibleLines, (int)text.size());

        for (int i = startLine; i < endLine; i++) {
            DrawTextEx(codeFont, TextFormat("%4d", i + 1), 
                       (Vector2){5, (float)(i - scrollOffset) * lineHeight}, fontSize, 1, GRAY);
            DrawTextEx(codeFont, text[i].c_str(), 
                       (Vector2){50, (float)(i - scrollOffset) * lineHeight}, fontSize, 1, LIGHTGRAY);
        }

        if (showCursor) {
            float cursorDrawX = MeasureTextEx(codeFont, text[cursorY].substr(0, cursorX).c_str(), fontSize, 1).x;
            float cursorDrawY = (cursorY - scrollOffset) * lineHeight;
            DrawRectangle(50 + cursorDrawX, cursorDrawY + 2, 2, fontSize - 4, WHITE);
        }

        // Draw scroll bar
        if (text.size() > maxVisibleLines) {
            float scrollbarHeight = (float)screenHeight * maxVisibleLines / text.size();
            float scrollbarY = (float)scrollOffset / text.size() * screenHeight;

            DrawRectangle(screenWidth - 10, 0, 10, screenHeight, (Color){50, 50, 50, 255});
            DrawRectangle(screenWidth - 10, scrollbarY, 10, scrollbarHeight, LIGHTGRAY);
        }

        EndDrawing();
    }

    UnloadFont(codeFont);
    CloseWindow();

    return 0;
}
