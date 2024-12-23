#include <fstream>
#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdexcept>

class EditorState {
private:
    std::string m_fileName;
    std::string m_filePath;
    std::vector<std::string> m_textContent;
    bool m_isSaved;
    bool m_isCommandPaletteOpen;
    bool m_shouldClose;

public:
    EditorState() 
        : m_fileName("")
        , m_filePath("")
        , m_textContent()
        , m_isSaved(false)
        , m_isCommandPaletteOpen(false)
        , m_shouldClose(false) {}

    const std::string& getFileName() const { return m_fileName; }
    void setFileName(const std::string& name) { m_fileName = name; }
    
    const std::string& getFilePath() const { return m_filePath; }
    void setFilePath(const std::string& path) { m_filePath = path; }
    
    std::vector<std::string>& getTextContent() { return m_textContent; }
    const std::vector<std::string>& getTextContent() const { return m_textContent; }
    
    bool isSaved() const { return m_isSaved; }
    void setSaved(bool saved) { m_isSaved = saved; }
    
    bool isCommandPaletteOpen() const { return m_isCommandPaletteOpen; }
    void setCommandPaletteOpen(bool open) { m_isCommandPaletteOpen = open; }
    
    bool shouldClose() const { return m_shouldClose; }
    void setShouldClose(bool close) { m_shouldClose = close; }
};

EditorState g_editorState;

void saveFile(const std::string& fileName) {
    std::string fileToSave = fileName.empty() ? g_editorState.getFilePath() : fileName;
    
    try {
        std::ofstream file(fileToSave, std::ios::out);
        if (!file) {
            throw std::runtime_error("Failed to open file for writing: " + fileToSave);
        }

        const auto& content = g_editorState.getTextContent();
        for (const auto& line : content) {
            file << line << "\n";
        }
        g_editorState.setSaved(true);
    } catch (const std::exception& e) {
        std::cerr << "Error saving file: " << e.what() << std::endl;
    }
}

void openAndReadFile(const std::string& filePath) {
    try {
        if (filePath.empty()) {
            g_editorState.setFileName("New File");
            g_editorState.setFilePath("./untitled.txt");
            g_editorState.getTextContent().clear();
            g_editorState.getTextContent().push_back("");
            return;
        }

        std::ifstream inputFile(filePath);
        if (!inputFile) {
            throw std::runtime_error("Failed to open file for reading: " + filePath);
        }

        std::vector<std::string> fileContent;
        std::string currentLine;
        while (std::getline(inputFile, currentLine)) {
            fileContent.push_back(currentLine);
        }

        if (fileContent.empty()) {
            fileContent.push_back("");
        }

        g_editorState.getTextContent() = std::move(fileContent);
        g_editorState.setFilePath(filePath);
        g_editorState.setFileName(filePath.substr(2));
    } catch (const std::exception& e) {
        std::cerr << "Error opening file: " << e.what() << std::endl;
        openAndReadFile("");
    }
}

std::pair<std::string, std::string> parseCommand(const std::string& commandString) {
    size_t spacePos = commandString.find(' ');
    if (spacePos == std::string::npos) {
        return {commandString, ""};
    }
    return {
        commandString.substr(0, spacePos),
        commandString.substr(spacePos + 1)
    };
}

void executeCommand(const std::string& command) {
    auto [cmd, args] = parseCommand(command);

    try {
        if (cmd == "open") {
            openAndReadFile(args);
        } else if (cmd == "save") {
            if (!args.empty()) {
                saveFile(args);
                g_editorState.setFileName(args);
                g_editorState.setFilePath("./" + args);
            } else {
                saveFile("");
            }
        } else if (cmd == "exit") {
            g_editorState.setShouldClose(true);
        } else {
            std::cerr << "Goffy ahh command! " << cmd << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Some fucky wacky happened" << e.what() << std::endl;
    }
}

//TODO: make a configuration file and tuck in all editables in that
struct EditorColors {
    const Color backgroundColor{20, 40, 50, 255};
    const Color scrollBarColor{180, 180, 180, 255};
    const Color scrollBarBackgroundColor{50, 50, 50, 255};
    const Color highlightLineColor{50, 80, 90, 180};
    const Color lineNumberColor{150, 150, 150, 255};
    const Color textColor{200, 200, 200, 255};
    const Color cursorColor{255, 255, 255, 255};
};

int main(int argc, char* argv[]) {
    constexpr int kInitialScreenWidth = 800;
    constexpr int kInitialScreenHeight = 600;
    constexpr int kFontSize = 22;
    constexpr float kKeyHoldDelay = 0.3f;
    constexpr int kLineSpacing = 5;
    
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(kInitialScreenWidth, kInitialScreenHeight, "nim");
    SetExitKey(KEY_NULL);

    if (argc > 1) {
        openAndReadFile(argv[1]);
    } else {
        openAndReadFile("");
    }

    int cursorX = 0;
    int cursorY = 0;
    float blinkTime = 0.0f;
    bool showCursor = true;
    bool isFullscreen = false;

    float backspaceTime = 0.0f;
    bool backspaceHeld = false;
    float arrowTime = 0.0f;
    bool arrowHeld = false;

    Font codeFont = LoadFontEx("./assets/font.ttf", kFontSize, 0, 0); //TODO: fix development and installation asset load
    int lineHeight = kFontSize + kLineSpacing;

    int scrollOffset = 0;
    int maxVisibleLines = kInitialScreenHeight / lineHeight;

    bool scrollbarDragging = false;
    float scrollbarY = 0;
    float scrollbarHeight = 0;
    float dragStartY = 0;

    std::string commandBuffer;
    EditorColors colors;

    SetTargetFPS(40);

    while (!g_editorState.shouldClose() && !WindowShouldClose()) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        maxVisibleLines = screenHeight / lineHeight;

        SetWindowTitle(g_editorState.getFileName().c_str());
        
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
                float scrollRatio = (float)g_editorState.getTextContent().size() / screenHeight;
                scrollOffset = std::clamp(scrollOffset + (int)(deltaY * scrollRatio), 
                                        0, 
                                        std::max(0, (int)g_editorState.getTextContent().size() - maxVisibleLines));
            }
        }

        cursorY -= (int)GetMouseWheelMove() * 3;
        cursorY = std::clamp(cursorY, 0, (int)g_editorState.getTextContent().size() - 1);

        if (cursorY < scrollOffset && !scrollbarDragging) {
            scrollOffset = cursorY;
        } else if (cursorY >= scrollOffset + maxVisibleLines && !scrollbarDragging) {
            scrollOffset = cursorY - maxVisibleLines + 1;
        }

        if (IsKeyPressed(KEY_F1)) {
            g_editorState.setCommandPaletteOpen(!g_editorState.isCommandPaletteOpen());
            commandBuffer.clear();
        }

        if (IsKeyDown(KEY_BACKSPACE)) {
            backspaceTime += GetFrameTime();
            showCursor = true;
            if (backspaceTime > kKeyHoldDelay || !backspaceHeld) {
                if (g_editorState.isCommandPaletteOpen()) {
                    if (!commandBuffer.empty()) {
                        commandBuffer.pop_back();
                    }
                } else {
                    auto& content = g_editorState.getTextContent();
                    if (cursorX > 0) {
                        content[cursorY].erase(cursorX - 1, 1);
                        cursorX--;
                    } else if (cursorY > 0) {
                        cursorX = content[cursorY - 1].size();
                        content[cursorY - 1] += content[cursorY];
                        content.erase(content.begin() + cursorY);
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
            if (arrowTime > kKeyHoldDelay || !arrowHeld) {
                auto& content = g_editorState.getTextContent();
                if (IsKeyDown(KEY_LEFT)) {
                    if (cursorX > 0) cursorX--;
                    else if (cursorY > 0) {
                        cursorY--;
                        cursorX = content[cursorY].size();
                    }
                } else if (IsKeyDown(KEY_RIGHT)) {
                    if (cursorX < content[cursorY].size()) cursorX++;
                    else if (cursorY < content.size() - 1) {
                        cursorY++;
                        cursorX = 0;
                    }
                } else if (IsKeyDown(KEY_UP)) {
                    if (cursorY > 0) {
                        cursorY--;
                        cursorX = std::min(cursorX, (int)content[cursorY].size());
                    }
                } else if (IsKeyDown(KEY_DOWN)) {
                    if (cursorY < content.size() - 1) {
                        cursorY++;
                        cursorX = std::min(cursorX, (int)content[cursorY].size());
                    }
                }
                arrowHeld = true;
            }
        } else {
            arrowTime = 0.0f;
            arrowHeld = false;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (g_editorState.isCommandPaletteOpen()) {
                executeCommand(commandBuffer);
                commandBuffer.clear();
                g_editorState.setCommandPaletteOpen(false);
            } else {
                showCursor = true;
                auto& content = g_editorState.getTextContent();
                std::string remaining = content[cursorY].substr(cursorX);
                content[cursorY] = content[cursorY].substr(0, cursorX);
                content.insert(content.begin() + cursorY + 1, remaining);
                cursorX = 0;
                cursorY++;
            }
        }

        if (IsKeyPressed(KEY_TAB)) {
            showCursor = true;
            g_editorState.getTextContent()[cursorY].insert(cursorX, "  "); //TODO: fix default tab value from raylib that is two. I want it four
            cursorX += 2;
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
                if (g_editorState.isCommandPaletteOpen()) {
                    commandBuffer += (char)key;
                } else {
                    auto content = g_editorState.getTextContent();
                    content[cursorY].insert(cursorX, 1, (char)key);
                    cursorX++;

                    float textWidth = MeasureTextEx(codeFont, content[cursorY].c_str(), kFontSize, 1).x;
                    if (textWidth > screenWidth - 50) {
                        std::string remaining = content[cursorY].substr(cursorX);
                        content[cursorY] = content[cursorY].substr(0, cursorX);
                        content.insert(content.begin() + cursorY + 1, remaining);
                        cursorY++;
                        cursorX = 0;
                    }
                }
            }
            key = GetCharPressed();
        }
        BeginDrawing();
        ClearBackground(colors.backgroundColor);

        DrawRectangle(0, 0, screenWidth, screenHeight, colors.backgroundColor);

        DrawRectangle(screenWidth - 12, 0, 12, screenHeight, colors.scrollBarBackgroundColor);

        const auto& content = g_editorState.getTextContent();
        scrollbarHeight = (float)screenHeight * maxVisibleLines / content.size();
        scrollbarY = (float)scrollOffset / content.size() * screenHeight;

        DrawRectangle(screenWidth - 12, scrollbarY, 12, scrollbarHeight, colors.scrollBarColor);

        int startLine = scrollOffset;
        int endLine = std::min(startLine + maxVisibleLines, (int)content.size());

        for (int lineNum = startLine; lineNum < endLine; lineNum++) {
            const int lineY = (lineNum - scrollOffset) * lineHeight;

            if (lineNum == cursorY) {
                DrawRectangle(50, lineY, screenWidth - 60 - 12, lineHeight, 
                            colors.highlightLineColor);
            }

            DrawTextEx(codeFont, 
                      TextFormat("%4d", lineNum + 1),
                      (Vector2){5, (float)lineY}, 
                      kFontSize, 
                      1, 
                      colors.lineNumberColor);

            DrawTextEx(codeFont, 
                      content[lineNum].c_str(),
                      (Vector2){50, (float)lineY}, 
                      kFontSize, 
                      1, 
                      colors.textColor);
        }

        if (showCursor && !g_editorState.isCommandPaletteOpen()) {
            float cursorDrawX = MeasureTextEx(codeFont, 
                                            content[cursorY].substr(0, cursorX).c_str(), 
                                            kFontSize, 
                                            1).x;
            float cursorDrawY = (cursorY - scrollOffset) * lineHeight;
            DrawRectangle(50 + cursorDrawX, 
                         cursorDrawY + 2, 
                         2, 
                         kFontSize - 2, 
                         colors.cursorColor);
        }

        if (g_editorState.isCommandPaletteOpen()) {
            Vector2 topLeft = {0, (float)screenHeight - lineHeight};
            Vector2 bottomRight = {(float)screenWidth, (float)screenHeight};
            
            constexpr Color kCommandPaletteGradientTop = {100, 30, 40, 255};
            constexpr Color kCommandPaletteGradientBottom = {150, 50, 60, 255};
            DrawRectangleGradientV(topLeft.x, 
                                 topLeft.y, 
                                 bottomRight.x, 
                                 bottomRight.y, 
                                 kCommandPaletteGradientTop, 
                                 kCommandPaletteGradientBottom);

            DrawTextEx(codeFont, 
                      commandBuffer.c_str(), 
                      (Vector2){10, (float)(screenHeight - lineHeight + 5)}, 
                      kFontSize, 
                      1, 
                      LIGHTGRAY);

            constexpr Color kCommandPaletteBorderColor = {180, 100, 100, 255};
            DrawRectangleLinesEx(
                (Rectangle){
                    0, 
                    (float)screenHeight - lineHeight, 
                    (float)screenWidth, 
                    (float)lineHeight
                }, 
                3, 
                kCommandPaletteBorderColor);

            const std::vector<std::string> suggestions; //TODO: implement suggestions and completion.
            const int selectedSuggestionIndex = 0;

            for (size_t i = 0; i < suggestions.size(); ++i) {
                Color suggestionColor = (i == selectedSuggestionIndex) 
                    ? Color{255, 255, 255, 255} 
                    : Color{200, 200, 200, 255};

                DrawTextEx(codeFont, 
                          suggestions[i].c_str(), 
                          (Vector2){10, (float)(screenHeight - lineHeight * (i + 2))}, 
                          kFontSize, 
                          1, 
                          suggestionColor);
            }
        }

        EndDrawing();
    }

    UnloadFont(codeFont);
    CloseWindow();
    return 0;
}
