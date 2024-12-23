#include <fstream>
#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

struct State {
	std::string name; 
	std::string filePath; //TODO: make separate definition for name and file path.
    std::vector<std::string> text;
	bool isSaved; //TODO: warn users on exiting on unsaved files
	bool isCommandPallateOpen;
	bool shouldClose;
};

State state{
	"",
	"",
	std::vector<std::string>(),
	false,
	false,
	false,
};

int save(std::string name){
	if (name == ""){
		name = state.filePath;
	}

	//TODO: catch errors
	std::ofstream file(name,std::ios::out );
	for(std::string line: state.text){
		file << line << "\n";
	}
	file.close();
	return 0;

}

int openAndScanFile(std::string path){
	if (path == ""){
		state.name = "New File";
		state.filePath = "./untitled.txt";
		state.text.push_back("");
		return 0;
	}

	//TODO: catch errors
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
	state.name = path.substr(2);
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

	//TODO: inform user about completion of command
	if (cmd == "open"){
		openAndScanFile(command.substr(5));
	}else if(cmd == "save"){
		if (command.length() > 5){
			save(command.substr(5));
			state.name = command.substr(5);
			state.filePath = "./" + state.name;
		}else{
			save("");
		}
	}else if (cmd == "exit"){
		state.shouldClose = true;
	}else {
		//TODO: warn users about not getting the command right.
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

    Font codeFont = LoadFontEx("./assets/font.ttf", fontSize, 0, 0);
    int lineHeight = fontSize + 5;

    int scrollOffset = 0;
    int maxVisibleLines = screenHeight / lineHeight;

    bool scrollbarDragging = false;
    float scrollbarY = 0;
    float scrollbarHeight = 0;
    float dragStartY = 0;

    bool isCommandPallateOpen = false;
    std::string command = "";

	//SECTION: COLORS
	Color BackgroundColor = {20, 40, 50, 255};
	Color ScrollBarColor  = {180, 180, 180, 255};
	Color ScrollBarBackgroundColor  = {50, 50, 50, 255};
	Color HighlightLineColor = {50, 80, 90, 180};
	Color LineNumberColor = {150, 150, 150, 255};
	Color TextColor = {200, 200, 200, 255};
	Color CursorColor = {255, 255, 255, 255};

    SetTargetFPS(40);

    while (!state.shouldClose && !WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        maxVisibleLines = screenHeight / lineHeight;

		SetWindowTitle(state.name.c_str());
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
            state.text[cursorY].insert(cursorX, "  ");
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
		ClearBackground(BackgroundColor);  

		DrawRectangle(0, 0, screenWidth, screenHeight, BackgroundColor); 

		DrawRectangle(screenWidth - 12, 0, 12, screenHeight, ScrollBarBackgroundColor);  
		scrollbarHeight = (float)screenHeight * maxVisibleLines / state.text.size();
		scrollbarY = (float)scrollOffset / state.text.size() * screenHeight;
		DrawRectangle(screenWidth - 12, scrollbarY, 12, scrollbarHeight, ScrollBarColor);  

		int startLine = scrollOffset;
		int endLine = std::min(startLine + maxVisibleLines, (int)state.text.size());

		for (int i = startLine; i < endLine; i++) {
			if (i == cursorY) {
				DrawRectangle(50, (i - scrollOffset) * lineHeight, screenWidth - 60 - 12, lineHeight, HighlightLineColor);
			}

			DrawTextEx(codeFont, TextFormat("%4d", i + 1),
					   (Vector2){5, (float)(i - scrollOffset) * lineHeight}, fontSize, 1, LineNumberColor);

			DrawTextEx(codeFont, state.text[i].c_str(),
					   (Vector2){50, (float)(i - scrollOffset) * lineHeight}, fontSize, 1, TextColor);
		}

		if (showCursor && !isCommandPallateOpen) {
			float cursorDrawX = MeasureTextEx(codeFont, state.text[cursorY].substr(0, cursorX).c_str(), fontSize, 1).x;
			float cursorDrawY = (cursorY - scrollOffset) * lineHeight;
			DrawRectangle(50 + cursorDrawX, cursorDrawY + 2, 2, fontSize - 2, CursorColor);  
		}

		if (isCommandPallateOpen) {
			Vector2 topLeft = {0, (float)screenHeight - lineHeight};
			Vector2 bottomRight = {(float)screenWidth, (float)screenHeight};
			DrawRectangleGradientV(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y, (Color){100, 30, 40, 255}, (Color){150, 50, 60, 255}); 

			DrawTextEx(codeFont, command.c_str(), (Vector2){10, (float)(screenHeight - lineHeight + 5)}, fontSize, 1, LIGHTGRAY);

			DrawRectangleLinesEx((Rectangle){0, (float)screenHeight - lineHeight, (float)screenWidth, (float)lineHeight}, 3, (Color){180, 100, 100, 255});  

			std::vector<std::string> suggestions = {};
			int selectedSuggestionIndex = 0;
			for (size_t i = 0; i < suggestions.size(); ++i) {
				Color suggestionColor = (i == selectedSuggestionIndex) ? (Color){255, 255, 255, 255} : (Color){200, 200, 200, 255};

				DrawTextEx(codeFont, suggestions[i].c_str(), (Vector2){10, (float)(screenHeight - lineHeight * (i + 2))}, fontSize, 1, suggestionColor);
			}
		}

		EndDrawing();

    }

    UnloadFont(codeFont);
    CloseWindow();
    return 0;
}
