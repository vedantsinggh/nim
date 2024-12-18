#include "raylib.h"
#include <string>
#include <vector>

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "nim");

    std::vector<std::string> text;
    text.push_back("");

    int fontSize = 22; 
    int cursorX = 0;
    int cursorY = 0;
    float blinkTime = 0.0f;
    bool showCursor = true;

    bool isFullscreen = false;

    float backspaceTime = 0.0f;
    bool backspaceHeld = false;

    Font codeFont = LoadFontEx("font.ttf", fontSize, 0, 0); 
    int lineHeight = fontSize + 5;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        blinkTime += GetFrameTime();
        if (blinkTime >= 0.5f) {
            showCursor = !showCursor;
            blinkTime = 0.0f;
        }

        if (IsKeyDown(KEY_BACKSPACE)) {
            backspaceTime += GetFrameTime();
            if (backspaceTime > 0.2f || !backspaceHeld) {
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

        if (IsKeyPressed(KEY_ENTER)) {
            std::string remaining = text[cursorY].substr(cursorX);
            text[cursorY] = text[cursorY].substr(0, cursorX);
            text.insert(text.begin() + cursorY + 1, remaining);
            cursorX = 0;
            cursorY++;
        }

        if (IsKeyPressed(KEY_TAB)) {
            text[cursorY].insert(cursorX, "    ");
            cursorX += 4;
        }

        if (IsKeyPressed(KEY_LEFT)) {
            if (cursorX > 0) cursorX--;
            else if (cursorY > 0) {
                cursorY--;
                cursorX = text[cursorY].size();
            }
        } else if (IsKeyPressed(KEY_RIGHT)) {
            if (cursorX < text[cursorY].size()) cursorX++;
            else if (cursorY < text.size() - 1) {
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
		


		if (IsKeyPressed(KEY_F11)) {
			isFullscreen = !isFullscreen;
			if (isFullscreen) {
				ToggleFullscreen();
			} else {
				SetWindowSize(screenWidth, screenHeight);
				SetWindowPosition(100, 100);
			}
		}

		if (IsKeyPressed(KEY_F10)) {
			MinimizeWindow();
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
        ClearBackground(DARKGRAY);

        DrawRectangle(0, cursorY * lineHeight, screenWidth, lineHeight, (Color){50, 50, 50, 255});

        for (int i = 0; i < text.size(); i++) {
            DrawTextEx(codeFont, TextFormat("%4d", i + 1), (Vector2){ 5, i * lineHeight}, fontSize, 1, GRAY);
        }

        for (int i = 0; i < text.size(); i++) {
            DrawTextEx(codeFont, text[i].c_str(), (Vector2){50, i * lineHeight}, fontSize, 1, LIGHTGRAY);
        }

        if (showCursor) {
            float cursorDrawX = MeasureTextEx(codeFont, text[cursorY].substr(0, cursorX).c_str(), fontSize, 1).x;
            float cursorDrawY = cursorY * lineHeight;
            DrawRectangle(50 + cursorDrawX, cursorDrawY + 2, 2, fontSize - 4, WHITE);
        }

        EndDrawing();
    }

    UnloadFont(codeFont);
    CloseWindow();

    return 0;
}
