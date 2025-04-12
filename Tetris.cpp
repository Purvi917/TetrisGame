#include<iostream>
#include<vector>
#include<windows.h>
#include<conio.h>
#include<ctime>
#include<sstream>
#include<algorithm>
#include<fstream>

using namespace std;

// Game Board
const int WIDTH = 10;
const int HEIGHT = 20;
#define RED   "\033[41m  \033[0m"
#define GREEN "\033[42m  \033[0m"
#define YELLOW "\033[43m  \033[0m"
#define BLUE "\033[44m  \033[0m"
#define MAGENTA "\033[45m  \033[0m"
#define CYAN "\033[46m  \033[0m"
#define WHITE "\033[47m  \033[0m"
#define RESET "\033[0m" // Reset color

// Tetromino Shape
vector<vector<vector<int>>> tetrominoes = 
{
    {{1, 1, 1, 1}},               // I = straight line
    {{1, 1}, {1, 1}},             // O = square
    {{0, 1, 0}, {1, 1, 1}},       // T =
    {{0, 1, 1}, {1, 1, 0}},       // Z
    {{1, 1, 0}, {0, 1, 1}},       // S
    {{1, 0, 0}, {1, 1, 1}},       // J
    {{0, 0, 1}, {1, 1, 1}}        // L
};

class Tetris
{
    private:
        vector<vector<int>> grid;   // grid holds locked blocks' color index, -1 means empty
        int score, level, highScore, difficulty, speed;
        int fallInterval;   // ms delay b/w automatic falls
        pair<int,int> pos;   // top-left position of active piece
        vector<vector<int>> curPiece;
        int curColor;   // index for active piece's color
        HANDLE hConsole;
        bool gameOver;
        bool lifelineUsed = false;
        vector<string> colors;  // tetromino colors string

        vector<vector<int>> nextPiece;
        int nextColor;

        // Generates random next piece
        void generateNextPiece()
        {
            int index = rand() % tetrominoes.size();
            nextPiece = tetrominoes[index];
            nextColor = index;
        }

    public:
        Tetris()
        {
            void generateObstacles();
        }
        Tetris(int diff) : difficulty(diff)
        {
            grid = vector<vector<int>>(HEIGHT, vector<int>(WIDTH, -1));
            score = 0;
            level = 1;
            gameOver = false;
            fallInterval = 500;  //miliseconds
            speed = (difficulty == 2) ? 400 : (difficulty == 3) ? 300 : 500;

            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            highScore = loadHighScore();
            if (difficulty == 3) generateObstacles();

            // Hide the console cursor
            CONSOLE_CURSOR_INFO cursorInfo;
            GetConsoleCursorInfo( hConsole, &cursorInfo );
            cursorInfo.bVisible = false;
            SetConsoleCursorInfo( hConsole, &cursorInfo );

            // color string using emojis
            colors = {"🟥", "🟧", "🟨", "🟩", "🟦", "🟪", "⬜"};

            // Generate next piece than spawn current piece
            generateNextPiece();
            spawnPiece();
        }
        void generateObstacles() 
        {
            int obstacleRows = HEIGHT / 4;
            for (int i = HEIGHT - obstacleRows; i < HEIGHT; i++) 
            {
                for (int j = 0; j < WIDTH; j++) 
                {
                    if (rand() % 3 == 0) grid[i][j] = 1;
                }
            }
        }

        // Use Pre-generated next piece as current piece than generate next piece
        void spawnPiece() 
        {
            curPiece = nextPiece;  // Use pre-generated next piece
            curColor = nextColor;
            pos.first = 0; //top row
            pos.second = WIDTH / 2 - (int)curPiece[0].size() / 2; //piece is placed horizontally at the center of the grid.
        
            // Check for game over condition
            if (!isValidMove(pos.first, pos.second, curPiece)) 
            {
                gameOver = true;
                Beep(500, 300);  // Game over sound 500 Hz, 300 ms
            }
        
            // Generate next piece only after placing the current one
            generateNextPiece();
        }
        
        bool isValidMove (int newRow, int newCol, const vector<vector<int>> &piece)
        {
            for (int i = 0; i < piece.size(); i++)
            {
                for (int j = 0; j < piece[i].size(); j++)
                {
                    if (piece[i][j])
                    {
                        int r = newRow + i;
                        int c = newCol + j;
                        if (r < 0 || r >= HEIGHT || c < 0 || c >= WIDTH || grid[r][c] != -1)
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }

        // Rotate active piece clockwise 90 degree
        vector<vector<int>> rotatePieceMatrix(const vector<vector<int>> &piece)
        {
            int rows = piece.size(); //Gives the number of rows.
            int cols = piece[0].size();//Gives the number of columns.
            vector<vector<int>> rotated(cols, vector<int>(rows, 0));
            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    rotated[j][rows - 1 - i] = piece[i][j];
                }
            }
            return rotated;
        }

        void rotatePiece()
        {
            vector<vector<int>> rotated = rotatePieceMatrix(curPiece);
            int newHeight = rotated.size();
            int newWidth = rotated[0].size();
            int newRow = pos.first;
            int newCol = pos.second;

            if (newRow + newHeight > HEIGHT)  //bottom boundry
                newRow = HEIGHT - newHeight;
            if (newRow < 0)   //Top Boundary
                newRow = 0;
            if (newCol + newWidth > WIDTH)  //Right Boundary
                newCol = WIDTH - newWidth;
            if (newCol < 0)   //Left Boundary
                newCol = 0;
            if (isValidMove(newRow, newCol, rotated)){
                pos.first = newRow;
                pos.second = newCol;
                curPiece = rotated;
                Beep(600,100);   // Rotation sound
            }
        }
       
        //for horizontally moving
        void movePiece (int dx)
        {
            if (isValidMove(pos.first, pos.second + dx, curPiece)) //row same 
            {
                pos.second += dx; //update column
                Beep(300, 150);   // Move sound
            }
        }

        bool moveDown()
        {
            if (isValidMove(pos.first + 1, pos.second, curPiece)) //column same 
            {
                pos.first++; //update row
                return true;
            }
            return false;
        }

        void dropPiece()
        {
            while (isValidMove(pos.first + 1, pos.second, curPiece))
            {
                pos.first++;
            }
            Beep(800, 100);   // Hard drop sound
        }

        void lockPiece()
        {
            for (int i = 0; i < curPiece.size(); i++)
            {
                for (int j = 0; j < curPiece[i].size(); j++){
                    if (curPiece[i][j] && pos.first + i >= 0){
                        grid[pos.first + i][pos.second + j] = curColor; //Assigns the Tetromino’s current color 
                    }
                }
            }
            clearLines();
            spawnPiece();
        }

        void clearLines(){
            int LinesCleared = 0;
            for (int i = HEIGHT - 1; i >= 0; i--){
                bool full = true;
                for (int j = 0; j < WIDTH; j++){
                    if (grid[i][j] == -1){
                        full = false;
                        break;
                    }
                }
                if (full){
                    LinesCleared++;
                    grid.erase(grid.begin() + i);
                    grid.insert(grid.begin(), vector<int>(WIDTH, -1)); //top of the grid
                    i++;   // Re-check same row after shifting
                    Beep(1000, 150);   // Line clear sound
                }
            }

            score += LinesCleared * 100; //1 line = 100 points
            if (score >= level * 500){   //500 points = level 2
                level++;
                fallInterval = max (50, fallInterval - 50);
                Beep(1100, 150);   // Level up sound
            }
        }

        int loadHighScore() {
            ifstream file("highscore.txt");
            int hs = 0;
            if (file) file >> hs; //Checks whether the file stream was successfully opened.
            file.close();
            return hs;
        }
    
        void saveHighScore() {
            if (score > highScore) {
                highScore = score;
                ofstream file("highscore.txt");
                file << highScore;
                file.close();
            }
        }

        void useLifeline() {
            for (int i = HEIGHT - 1; i >= HEIGHT - 3; i--) {
                for (int j = 0; j < WIDTH; j++)
                    grid[i][j] = 0;
            }
            Beep(1200, 200);  
        }

        bool isGameOver()
        {
            return gameOver;
        }

        // Draw() builds complete frame (grid with borders, ghost piece, score/level, next piece preview)
        void drawBoard()
        {
            // Calculate ghost piece position
            pair<int,int> GhostPos = pos;
            while (isValidMove(GhostPos.first + 1, GhostPos.second, curPiece))
            {
                GhostPos.first++;
            }

            ostringstream frame;
            // Top border
            frame << "+";
            for (int j = 0; j < WIDTH * 2; j++)
                frame << "-";
            frame << "+\n";

            // Grid rows
            for (int i = 0; i < HEIGHT; i++)
            {
                frame << "|";
                for (int j = 0; j < WIDTH; j++)
                {
                    bool activeCell = false;
                    bool ghostCell = false;
                    int cellColor = -1;
                    for (int pi = 0; pi < curPiece.size(); pi++)
                    {
                        for (int pj = 0; pj < curPiece[pi].size(); pj++)
                        {
                            if (curPiece[pi][pj])
                            {
                                if (pos.first + pi == i && pos.second + pj == j)
                                {
                                    activeCell = true;
                                    cellColor = curColor;
                                }
                                if (GhostPos.first + pi == i && GhostPos.second + pj == j)
                                {
                                    ghostCell = true;
                                }
                            }
                        }
                    }
                    if (activeCell)
                        frame << colors[cellColor];
                    else if (grid[i][j] != -1)
                        frame << colors[grid[i][j]];
                    else if (ghostCell)
                        frame << "\033[2m" << colors[curColor] << "\033[22m";
                    else
                        frame << "\033[48;5;235m  \033[0m";
                }
                frame << "|\n";
            }

            // Bottom border
            frame << "+";
            for (int j = 0; j < WIDTH * 2; j++)
            {
                frame << "-";
            }
            frame << "+\n";

            // Score, Level and lifeline
            frame << "Score : " << score << "   " << "Level : " << level << "   " << "Highscore : " << highScore << "\n";
            frame << "Lifeline Available : " << (lifelineUsed ? "No" : "YES") << "\n";

            // Next piece preview
            frame << "\nNext Piece :\n";
            if (!nextPiece.empty())
            {
                frame << "+";
                for (int j = 0; j < nextPiece[0].size() * 2; j++)
                {
                    frame << "-";
                }
                frame << "+\n";
                for (int i = 0; i < nextPiece.size(); i++)
                {
                    frame << "|";
                    for (int j = 0; j < nextPiece[i].size(); j++)
                    {
                        if (nextPiece[i][j])
                        {
                            frame << colors[nextColor];
                        }
                        else
                        {
                            frame << "  ";
                        }
                    }
                    frame << "|\n";
                }
                frame << "+";
                for (int j = 0; j < nextPiece[0].size() * 2; j++)
                {
                    frame << "-";
                }
                frame << "+\n";
            }

            SetConsoleCursorPosition(hConsole, {0,0});
            cout << frame.str();
        }

        void handleInput()
        {
            if (_kbhit())
            {
                char key = _getch();
                if (key == 0 || key == -32)  // arrow keys
                {   
                    key = _getch();
                    if (key == 75)   // left key
                        movePiece(-1);
                    else if (key == 77)   // right key
                        movePiece(1);
                    else if (key == 80)   // down key
                    {    
                        if (!moveDown())
                        {
                            lockPiece();
                        }
                    }
                    else if (key == 72)   // up key
                        rotatePiece();
                }
                else if (key == 27)  // esc key  ,exit game
                {    
                    exit(0);
                }
                else if (key == 32)
                {    // hard drop - space bar
                    dropPiece();
                    lockPiece();
                }
                else if (key == '1' && !lifelineUsed) //1
                {
                    useLifeline(); 
                    lifelineUsed = true;
                }
            }
        }

        void play()
        {
            DWORD lastFall = GetTickCount();
            const int frameDelay = 16;    // 60 FPS
            while (!isGameOver())
            {
                handleInput();
                DWORD now = GetTickCount();
                if (now - lastFall >= (DWORD)fallInterval)
                {
                    if (!moveDown())
                        lockPiece();
                    lastFall = now;
                }
                drawBoard();
                Sleep(frameDelay);
            }
            drawBoard();
            saveHighScore();
            cout << "\nGame Over! Final Score : " << score << "\n";
        }
};

int main()
{
    srand(time(0));
    SetConsoleOutputCP(CP_UTF8);   // Without , the emojis and other non-ASCII characters might not display correctly.
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);   // to change the text attributes (e.g., colors)
    DWORD dwMode = 0;//windows.h ,sed here to store bitwise flags (binary settings) that control the behavior of the console
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode);
    while(true)
    {
        int difficulty;
        cout << "Select Difficulty (1-Easy, 2-Medium, 3-Hard): ";
        cin >> difficulty;

        Tetris game(difficulty);
        game.play();

        char choice;
     do
     {
        Tetris game;
        //game.play();
        cout << "\nGame Over! Press R to restart, ESC to exit...";
        while(true)
        {
            if (_kbhit())
            {
                char key = _getch();
                if (key == 'r' || key == 'R'){
                    choice = 'r';
                    break;
                }
                else if(key == 27)
                {    // ESC key
                    choice = 27;
                    cout << "Thanks for playing! Goodbye!" << endl;       
                    break;
                }
            }
        }
     }while (choice == 'r');
    }

    return  0;
}
