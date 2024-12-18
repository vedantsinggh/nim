#include "raylib.h"
#include <string>
#include <vector>

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "NIM Text Editor");

    std::vector<std::string> text; 
    text.push_back("");

    int fontSize = 35;
    int cursorX = 0; 
    int cursorY = 0; 
    float blinkTime = 0.0f;
    bool showCursor = true;

    Font customFont = LoadFont("./font.ttf");
    int sizeY = fontSize + 5; 

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        blinkTime += GetFrameTime();
        if (blinkTime >= 0.5f) {
            showCursor = !showCursor;
            blinkTime = 0.0f;
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (cursorX > 0) {
                text[cursorY].erase(cursorX - 1, 1);
                cursorX--;
            } else if (cursorY > 0) {
                cursorX = text[cursorY - 1].size();
                text[cursorY - 1] += text[cursorY];
                text.erase(text.begin() + cursorY);
                cursorY--;
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            std::string remaining = text[cursorY].substr(cursorX);
            text[cursorY] = text[cursorY].substr(0, cursorX);
            text.insert(text.begin() + cursorY + 1, remaining);
            cursorX = 0;
            cursorY++;
        }
        if (IsKeyPressed(KEY_LEFT)) {
            if (cursorX > 0) {
                cursorX--;
            } else if (cursorY > 0) {
                cursorY--;
                cursorX = text[cursorY].size();
            }
        } else if (IsKeyPressed(KEY_RIGHT)) {
            if (cursorX < text[cursorY].size()) {
                cursorX++;
            } else if (cursorY < text.size() - 1) {
                cursorY++;
                cursorX = 0;
            }
        } else if (IsKeyPressed(KEY_UP)) {
            if (cursorY > 0) {
                cursorY--;
                cursorX = (cursorX > text[cursorY].size()) ? text[cursorY].size() : cursorX;
            }
        } else if (IsKeyPressed(KEY_DOWN)) {
            if (cursorY < text.size() - 1) {
                cursorY++;
                cursorX = (cursorX > text[cursorY].size()) ? text[cursorY].size() : cursorX;
            }
        }
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) { 
                text[cursorY].insert(cursorX, 1, (char)key);
                cursorX++;
            }
            key = GetCharPressed();
        }
        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < text.size(); i++) {
            DrawTextEx(customFont, text[i].c_str(), (Vector2){20, i * sizeY}, fontSize, 1, WHITE);
        }

		if (showCursor) {
			float cursorDrawX = MeasureTextEx(customFont, text[cursorY].substr(0, cursorX).c_str(), fontSize, 1).x;
			float cursorDrawY = cursorY * sizeY;
			DrawRectangle(20 + cursorDrawX, cursorDrawY + 4, 2, fontSize - 8, WHITE); 
		}
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
