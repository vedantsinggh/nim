#include <ncurses.h>
#include <string>
#include <vector>

using namespace std;

class TextEditor {
public:
    TextEditor() {
        initscr();            
        raw();                
        keypad(stdscr, TRUE);
        noecho();             
        curs_set(1);          

        cursor_x = 0;
        cursor_y = 0;
        text_lines.push_back("");
    }

    ~TextEditor() {
        endwin();
    }

    void run() {
        int ch;
        while ((ch = getch()) != 27) { 
            process_input(ch);
            render();
        }
    }

private:
    int cursor_x, cursor_y;
    vector<string> text_lines;

    void process_input(int ch) {
        switch (ch) {
            case KEY_UP:
                if (cursor_y > 0) {
                    cursor_y--;
                }
                break;
            case KEY_DOWN:
                if (cursor_y < text_lines.size() - 1) {
                    cursor_y++;
                }
                break;
            case KEY_LEFT:
                if (cursor_x > 0) {
                    cursor_x--;
                }
                break;
            case KEY_RIGHT:
                if (cursor_x < text_lines[cursor_y].length()) {
                    cursor_x++;
                }
                break;
            case 10: 
                text_lines.insert(text_lines.begin() + cursor_y + 1, "");
                cursor_y++;
                cursor_x = 0;
                break;
            case 127: 
                if (cursor_x > 0) {
                    text_lines[cursor_y].erase(cursor_x - 1, 1);
                    cursor_x--;
                } else if (cursor_y > 0) {
                    cursor_x = text_lines[cursor_y - 1].length();
                    text_lines[cursor_y - 1] += text_lines[cursor_y];
                    text_lines.erase(text_lines.begin() + cursor_y);
                    cursor_y--;
                }
                break;
            default:
                if (ch >= 32 && ch <= 126) { 
                    text_lines[cursor_y].insert(cursor_x, 1, (char)ch);
                    cursor_x++;
                }
                break;
        }
    }

    void render() {
        clear(); 

        for (int i = 0; i < text_lines.size(); ++i) {
            mvprintw(i, 0, "%s", text_lines[i].c_str());
        }

        move(cursor_y, cursor_x);

        refresh(); 
    }
};

int main() {
    TextEditor editor;
    editor.run();
    return 0;
}
