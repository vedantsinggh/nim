#include <fstream>
#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

struct State {
	std::string name; 
	std::string filePath;
    std::vector<std::string> text;
	bool isSaved;
	bool isCommandPallateOpen;
};

State state{
	"",
	"",
	std::vector<std::string>(),
	false,
	false,
};

int save(std::string name){
	if (name == ""){
		name = state.filePath;
	}

	std::ofstream file(name,std::ios::out );
	for(std::string line: state.text){
		file << line << "\n";
	}

	return 0;

}

int openAndScanFile(std::string path){
	if (path == ""){
		state.text.push_back("");
		return 0;
	}
    std::ifstream file(path);
	state.text.clear();
    std::string line = "";
    while (std::getline(file, line)) {
        state.text.push_back(line);
    }

	if (state.text.size() == 0){
		state.text.push_back("");
	}

	state.filePath = path;
	file.close();
	return 0;
}


int executeCommand(std::string command) {
	std::string cmd = "";
	for (char c : command){
		if (c == 32){
			break;
		}else {
			cmd.push_back(c);
		}
	}

	if (cmd == "open"){
		std::cout << "opening " << command.substr(5) << "\n";
		openAndScanFile(command.substr(5));
	}else if("save"){
		save("");
	}else {
		std::cout << "type properly bitch ass nigga\n";
	}
    return 0;
}


int main(int argc, char* argv[]) {
    int screenWidth = 800;
    int screenHeight = 600;
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "nim");
    SetExitKey(KEY_NULL);

	if(argc > 1){
		openAndScanFile(argv[1]);
	}else{
		openAndScanFile("");
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

    bool scrollbarDragging = false;
    float scrollbarY = 0;
    float scrollbarHeight = 0;
    float dragStartY = 0;

    bool isCommandPallateOpen = false;
    std::string command = "";

    SetTargetFPS(40);

    while (!WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        maxVisibleLines = screenHeight / lineHeight;

        blinkTime += GetFrameTime();
        if (blinkTime >= 0.5f) {
            showCursor = !showCursor;
            blinkTime = 0.0f;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            float mouseX = GetMouseX();
            float mouseY = GetMouseY();
            if (mouseX > screenWidth - 10 && mouseX < screenWidth) {
                if (mouseY > scrollbarY && mouseY < scrollbarY + scrollbarHeight) {
                    scrollbarDragging = true;
                    dragStartY = mouseY;
                }
            }
            if (mouseX < screenWidth - 10) {
                cursorY = scrollOffset + (int)(mouseY / lineHeight);
            }
        }

        if (scrollbarDragging) {
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                scrollbarDragging = false;
            } else {
                float mouseY = GetMouseY();
                float deltaY = mouseY - dragStartY;
                dragStartY = mouseY;
                float scrollRatio = (float)state.text.size() / screenHeight;
                scrollOffset = std::clamp(scrollOffset + (int)(deltaY * scrollRatio), 0, std::max(0, (int)state.text.size() - maxVisibleLines));
            }
        }

        cursorY -= (int)GetMouseWheelMove() * 3;
        if (cursorY < 0) cursorY = 0;
        if (cursorY > state.text.size()) cursorY = state.text.size() - 1;

        if (cursorY < scrollOffset && !scrollbarDragging) {
            scrollOffset = cursorY;
        } else if (cursorY >= scrollOffset + maxVisibleLines && !scrollbarDragging) {
            scrollOffset = cursorY - maxVisibleLines + 1;
        }

        if (IsKeyPressed(KEY_F1)) {
            isCommandPallateOpen = !isCommandPallateOpen;
            command = "";
        }

        if (IsKeyDown(KEY_BACKSPACE)) {
            backspaceTime += GetFrameTime();
            showCursor = true;
            if (backspaceTime > keyHeldTime || !backspaceHeld) {
                if (isCommandPallateOpen) {
                    if (command.length() > 0) command.erase(command.length() - 1, 1);
                } else {
                    if (cursorX > 0) {
                        state.text[cursorY].erase(cursorX - 1, 1);
                        cursorX--;
                    } else if (cursorY > 0) {
                        cursorX = state.text[cursorY - 1].size();
                        state.text[cursorY - 1] += state.text[cursorY];
                        state.text.erase(state.text.begin() + cursorY);
                        cursorY--;
                    }
                }
                backspaceHeld = true;
            }
        } else {
            backspaceTime = 0.0f;
            backspaceHeld = false;
        }

        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN)) {
            showCursor = true;
            arrowTime += GetFrameTime();
            if (arrowTime > keyHeldTime || !arrowHeld) {
                if (IsKeyDown(KEY_LEFT)) {
                    if (cursorX > 0) cursorX--;
                    else if (cursorY > 0) {
                        cursorY--;
                        cursorX = state.text[cursorY].size();
                    }
                } else if (IsKeyDown(KEY_RIGHT)) {
                    if (cursorX < state.text[cursorY].size()) cursorX++;
                    else if (cursorY < state.text.size() - 1) {
                        cursorY++;
                        cursorX = 0;
                    }
                } else if (IsKeyDown(KEY_UP)) {
                    if (cursorY > 0) {
                        cursorY--;
                        cursorX = (cursorX > state.text[cursorY].size()) ? state.text[cursorY].size() : cursorX;
                    }
                } else if (IsKeyDown(KEY_DOWN)) {
                    if (cursorY < state.text.size() - 1) {
                        cursorY++;
                        cursorX = (cursorX > state.text[cursorY].size()) ? state.text[cursorY].size() : cursorX;
                    }
                }
                arrowHeld = true;
            }
        } else {
            arrowTime = 0.0f;
            arrowHeld = false;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (isCommandPallateOpen) {
                executeCommand(command);
                command = "";
                isCommandPallateOpen = false;
            } else {
                showCursor = true;
                std::string remaining = state.text[cursorY].substr(cursorX);
                state.text[cursorY] = state.text[cursorY].substr(0, cursorX);
                state.text.insert(state.text.begin() + cursorY + 1, remaining);
                cursorX = 0;
                cursorY++;
            }
        }

        if (IsKeyPressed(KEY_TAB)) {
            showCursor = true;
            state.text[cursorY].insert(cursorX, "    ");
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
                if (isCommandPallateOpen) {
                    command.insert(command.length(), 1, (char)key);
                } else {
                    state.text[cursorY].insert(cursorX, 1, (char)key);
                    cursorX++;
                }
                float textWidth = MeasureTextEx(codeFont, state.text[cursorY].c_str(), fontSize, 1).x;

                if (textWidth > screenWidth - 50) {
                    std::string remaining = state.text[cursorY].substr(cursorX);
                    state.text[cursorY] = state.text[cursorY].substr(0, cursorX);
                    state.text.insert(state.text.begin() + cursorY + 1, remaining);
                    cursorY++;
                    cursorX = 0;
                }
            }
            key = GetCharPressed();
        }

        BeginDrawing();
        ClearBackground((Color){1, 31, 38, 255});

        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){1, 31, 38, 255});

        scrollbarHeight = (float)screenHeight * maxVisibleLines / state.text.size();
        scrollbarY = (float)scrollOffset / state.text.size() * screenHeight;

        int startLine = scrollOffset;
        int endLine = std::min(startLine + maxVisibleLines, (int)state.text.size());

        for (int i = startLine; i < endLine; i++) {
            if (i == cursorY) {
                DrawRectangle(50, (i - scrollOffset) * lineHeight, screenWidth - 60, lineHeight, (Color){2, 81, 89, 255});
            }

            DrawTextEx(codeFont, TextFormat("%4d", i + 1),
                       (Vector2){5, (float)(i - scrollOffset) * lineHeight}, fontSize, 1, GRAY);
            DrawTextEx(codeFont, state.text[i].c_str(),
                       (Vector2){50, (float)(i - scrollOffset) * lineHeight}, fontSize, 1, LIGHTGRAY);
        }

        if (showCursor && !isCommandPallateOpen) {
            float cursorDrawX = MeasureTextEx(codeFont, state.text[cursorY].substr(0, cursorX).c_str(), fontSize, 1).x;
            float cursorDrawY = (cursorY - scrollOffset) * lineHeight;
            DrawRectangle(50 + cursorDrawX, cursorDrawY + 2, 2, fontSize - 2, LIGHTGRAY);
        }

        DrawRectangle(screenWidth - 10, 0, 10, screenHeight, GRAY);
        DrawRectangle(screenWidth - 10, scrollbarY, 10, scrollbarHeight, WHITE);

        if (isCommandPallateOpen) {
            DrawRectangle(0, screenHeight - lineHeight, screenWidth, lineHeight, MAROON );
            DrawTextEx(codeFont, command.c_str(), (Vector2){5, (float)(screenHeight - lineHeight + 2)}, fontSize, 1, LIGHTGRAY);
        }

        EndDrawing();
    }

    UnloadFont(codeFont);
    CloseWindow();
    return 0;
}
