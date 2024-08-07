#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_CS     14
#define OLED_DC     27
#define OLED_RESET  26
#define OLED_MOSI   25
#define OLED_CLK    13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

const int BOARD_WIDTH = 32;
const int BOARD_HEIGHT = 16;
const int BLOCK_SIZE = 4;

bool board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

struct Tetromino {
  bool shape[4][4];
  int x, y;
};

Tetromino currentPiece;

const Tetromino TETROMINOES[] = {
  {{{1,1,1,1}}, 0, 0}, // I
  {{{1,1},{1,1}}, 0, 0}, // O
  {{{1,1,1},{0,1,0}}, 0, 0}, // T
  {{{1,1,1},{1,0,0}}, 0, 0}, // L
  {{{1,1,1},{0,0,1}}, 0, 0}, // J
  {{{1,1,0},{0,1,1}}, 0, 0}, // S
  {{{0,1,1},{1,1,0}}, 0, 0}  // Z
};

unsigned long lastMoveTime = 0;
const unsigned long moveDelay = 500; // Move every 500ms

void setup() {
  Serial.begin(115200);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.display();
  delay(2000);
  display.clearDisplay();
  
  randomSeed(analogRead(0));
  spawnNewPiece();
}

void loop() {
  handleInput();
  
  unsigned long currentTime = millis();
  if (currentTime - lastMoveTime >= moveDelay) {
    movePiece(1, 0);
    lastMoveTime = currentTime;
  }
  
  display.clearDisplay();
  drawBoard();
  drawPiece();
  display.display();
}

void handleInput() {
  if (Serial.available()) {
    char input = Serial.read();
    switch (input) {
      case 'w':
        rotatePiece();
        break;
      case 'a':
        movePiece(0, -1);
        break;
      case 's':
        movePiece(0, 1);
        break;
      case 'd':
        movePiece(1, 0);
        break;
    }
  }
}

void spawnNewPiece() {
  currentPiece = TETROMINOES[random(7)];
  currentPiece.x = 0;
  currentPiece.y = BOARD_HEIGHT / 2 - 2;
}

void drawBoard() {
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    for (int x = 0; x < BOARD_WIDTH; x++) {
      if (board[y][x]) {
        display.fillRect(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, SSD1306_WHITE);
      }
    }
  }
}

void drawPiece() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentPiece.shape[y][x]) {
        display.fillRect((currentPiece.x + x) * BLOCK_SIZE, (currentPiece.y + y) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, SSD1306_WHITE);
      }
    }
  }
}

void movePiece(int dx, int dy) {
  currentPiece.x += dx;
  currentPiece.y += dy;
  
  if (checkCollision()) {
    currentPiece.x -= dx;
    currentPiece.y -= dy;
    if (dx > 0) { // Only place the piece if it was moving right
      placePiece();
      clearLines();
      spawnNewPiece();
      if (checkCollision()) {
        resetGame();
      }
    }
  }
}

void rotatePiece() {
  Tetromino rotated = currentPiece;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      rotated.shape[x][3-y] = currentPiece.shape[y][x];
    }
  }
  
  currentPiece = rotated;
  if (checkCollision()) {
    // Undo rotation if it causes a collision
    currentPiece = rotated;
  }
}

bool checkCollision() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentPiece.shape[y][x]) {
        int boardX = currentPiece.x + x;
        int boardY = currentPiece.y + y;
        if (boardX < 0 || boardX >= BOARD_WIDTH || boardY < 0 || boardY >= BOARD_HEIGHT || board[boardY][boardX]) {
          return true;
        }
      }
    }
  }
  return false;
}

void placePiece() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentPiece.shape[y][x]) {
        board[currentPiece.y + y][currentPiece.x + x] = true;
      }
    }
  }
}

void clearLines() {
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    bool lineComplete = true;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      if (!board[y][x]) {
        lineComplete = false;
        break;
      }
    }
    if (lineComplete) {
      for (int yy = y; yy > 0; yy--) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
          board[yy][x] = board[yy-1][x];
        }
      }
      for (int x = 0; x < BOARD_WIDTH; x++) {
        board[0][x] = false;
      }
    }
  }
}

void resetGame() {
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    for (int x = 0; x < BOARD_WIDTH; x++) {
      board[y][x] = false;
    }
  }
  spawnNewPiece();
}