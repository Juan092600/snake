#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

using namespace std;

const int width = 40;
const int height = 25;
int x, y, fruitX, fruitY, score;
int tailX[1000], tailY[1000];
int nTail;
bool gameOver;
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

// Terminal raw input handling
termios orig_term;

void initTerminal() {
    tcgetattr(STDIN_FILENO, &orig_term);
    termios raw = orig_term;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void resetTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}

// Non-blocking input
bool kbhit() {
    timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, nullptr, nullptr, &tv) > 0;
}

char getch() {
    char buf = 0;
    if (read(STDIN_FILENO, &buf, 1) < 0) perror("read()");
    return buf;
}

// Game setup
void Setup() {
    gameOver = false;
    dir = STOP;
    x = width / 2;
    y = height / 2;
    fruitX = rand() % width;
    fruitY = rand() % height;
    score = 0;
    nTail = 0;
}

// Draw game state
void Draw() {
    // Move cursor to top-left instead of clearing screen
    cout << "\033[H";

    for (int i = 0; i < width + 2; i++) cout << "⬛";
    cout << endl;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (j == 0) cout << "⬛";

            if (i == y && j == x)
                cout << "🟩"; // Snake head
            else if (i == fruitY && j == fruitX)
                cout << "🍎"; // Fruit
            else {
                bool printTail = false;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        cout << "🟢"; // Snake body
                        printTail = true;
                        break;
                    }
                }
                if (!printTail)
                    cout << "  ";
            }

            if (j == width - 1) cout << "⬛";
        }
        cout << endl;
    }

    for (int i = 0; i < width + 2; i++) cout << "⬛";
    cout << endl;

    cout << "Score: " << score << endl;
}

// Handle keyboard input
void Input() {
    if (kbhit()) {
        char key = getch();
        switch (key) {
            case 'a': dir = LEFT; break;
            case 'd': dir = RIGHT; break;
            case 'w': dir = UP; break;
            case 's': dir = DOWN; break;
            case 'x': gameOver = true; break;
        }
    }
}

// Game logic
void Logic() {
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;
    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (dir) {
        case LEFT:  x--; break;
        case RIGHT: x++; break;
        case UP:    y--; break;
        case DOWN:  y++; break;
        default: break;
    }

    // Collisions
    if (x < 0 || x >= width || y < 0 || y >= height)
        gameOver = true;

    for (int i = 0; i < nTail; i++)
        if (tailX[i] == x && tailY[i] == y)
            gameOver = true;

    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = rand() % width;
        fruitY = rand() % height;
        nTail++;
    }
}

// Main game loop with replay option
int main() {
    initTerminal();
    atexit(resetTerminal);

    char choice;
    do {
        Setup();
        while (!gameOver) {
            Draw();
            Input();
            Logic();
            usleep(100000); // Adjust speed (microseconds)
        }
    
        cout << "💀 Game Over! Final Score: " << score << endl;
        cout << "🔁 Play again? (y/n): ";
        cout.flush(); // Flush output so it prints immediately
        choice = getch(); // Get key press
        cout << choice << endl; // Optional: Show what key was pressed for debugging
    
    } while (choice == 'y' || choice == 'Y');

    cout << "👋 Thanks for playing!\n";
    return 0;
}