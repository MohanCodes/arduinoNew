#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 50
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define JUMP_BUTTON A2
#define DUCK_BUTTON A1
#define BUZZ 3

// Dinosaurio parado
const unsigned char DINO_BITMAP[] PROGMEM = {
  0b000011110,
  0b000111111,
  0b001101111,
  0b011111111,
  0b111111000,
  0b111111110,
  0b111111000,
  0b111111000,
  0b111111110,
  0b111111010,
  0b111111010,
  0b111111000,
  0b110011000,
  0b110011000,
  0b111011100
};

// Dinosaurio agachado
const unsigned char DINO_DUCK_BITMAP[] PROGMEM = {
  0b000000000,
  0b000111000,
  0b001111100,
  0b011111110,
  0b111111111,
  0b111001111,
  0b111001111,
  0b111111111
};

// Pájaro (7x5)
const unsigned char BIRD_BITMAP[] PROGMEM = {
  0b00100000,
  0b01111100,
  0b01111110,
  0b00100000,
  0b01000000
};

const int DINO_WIDTH = 9;
const int DINO_HEIGHT = 15;
const int DINO_DUCK_HEIGHT = 8;
const int GROUND_HEIGHT = 5;
const int OBSTACLE_WIDTH = 3;
const int OBSTACLE_HEIGHT = 8;

int dinoY;
int obstacleX;
bool isJumping = false;
bool isDucking = false;
int jumpPhase = 0;
const int JUMP_HEIGHT = 10;
const int JUMP_DURATION = 6;
unsigned long score = 0;
int gameSpeed = 50;

enum ObstacleType { CACTUS, BIRD };
ObstacleType obstacleType = CACTUS;

void setup() {
  pinMode(JUMP_BUTTON, INPUT_PULLUP);
  pinMode(DUCK_BUTTON, INPUT_PULLUP);
  pinMode(BUZZ, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;
  obstacleX = SCREEN_WIDTH;

  randomSeed(analogRead(0));
  displayIntro();
}

void loop() {
  bool jumpPressed = analogRead(JUMP_BUTTON) < 100;
  bool duckPressed = analogRead(DUCK_BUTTON) < 100;

  if (jumpPressed && !isJumping && !duckPressed) {
    isJumping = true;
    jumpPhase = 0;
    playTone(523, 30); // Sonido al saltar
  }

  isDucking = duckPressed && !isJumping;

  updateGame();
  drawGame();
  display.display();

  // Aumenta la velocidad según el puntaje
  gameSpeed = max(10, 50 - score / 50);

  delay(gameSpeed);

  score++;
  // Sonido cada 100 puntos
  if (score % 100 == 0 && score != 0) {
    playTone(1046, 50);
    playTone(1567, 200);
  }
}

void updateGame() {
  if (isJumping) {
    if (jumpPhase < JUMP_DURATION) {
      dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT - (JUMP_HEIGHT * jumpPhase / JUMP_DURATION);
    } else if (jumpPhase < JUMP_DURATION * 2) {
      dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT - JUMP_HEIGHT;
    } else if (jumpPhase < JUMP_DURATION * 3) {
      dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT - (JUMP_HEIGHT * (JUMP_DURATION * 3 - jumpPhase) / JUMP_DURATION);
    } else {
      isJumping = false;
    }
    jumpPhase++;
  }

  if (!isJumping && !isDucking) {
    dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;
  }

  obstacleX -= 2;
  if (obstacleX < -OBSTACLE_WIDTH) {
    obstacleX = SCREEN_WIDTH;
    obstacleType = (random(0, 2) == 0) ? CACTUS : BIRD;
  }

  int currentDinoHeight = isDucking ? DINO_DUCK_HEIGHT : DINO_HEIGHT;

  // Verifica colisión
  if (obstacleType == CACTUS) {
    if (obstacleX < DINO_WIDTH &&
        obstacleX + OBSTACLE_WIDTH > 0 &&
        dinoY + currentDinoHeight > SCREEN_HEIGHT - GROUND_HEIGHT - OBSTACLE_HEIGHT) {
      gameOver();
    }
  } else if (obstacleType == BIRD) {
    int birdY = SCREEN_HEIGHT - GROUND_HEIGHT - OBSTACLE_HEIGHT - 10;
    if (obstacleX < DINO_WIDTH &&
        obstacleX + 7 > 0 &&
        !isDucking &&
        dinoY + currentDinoHeight > birdY) {
      gameOver();
    }
  }
}

void drawGame() {
  display.clearDisplay();

  // Dino
  if (isDucking) {
    for (int y = 0; y < DINO_DUCK_HEIGHT; y++) {
      for (int x = 0; x < DINO_WIDTH; x++) {
        if (pgm_read_byte(&DINO_DUCK_BITMAP[y]) & (1 << (7 - x))) {
          display.drawPixel(x, SCREEN_HEIGHT - GROUND_HEIGHT - DINO_DUCK_HEIGHT + y, SSD1306_WHITE);
        }
      }
    }
  } else {
    for (int y = 0; y < DINO_HEIGHT; y++) {
      for (int x = 0; x < DINO_WIDTH; x++) {
        if (pgm_read_byte(&DINO_BITMAP[y]) & (1 << (7 - x))) {
          display.drawPixel(x, dinoY + y, SSD1306_WHITE);
        }
      }
    }
  }

  // Obstáculo
  if (obstacleType == CACTUS) {
    display.fillRect(obstacleX, SCREEN_HEIGHT - GROUND_HEIGHT - OBSTACLE_HEIGHT, OBSTACLE_WIDTH, OBSTACLE_HEIGHT, SSD1306_WHITE);
  } else if (obstacleType == BIRD) {
    int birdY = SCREEN_HEIGHT - GROUND_HEIGHT - OBSTACLE_HEIGHT - 10;
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 7; x++) {
        if (pgm_read_byte(&BIRD_BITMAP[y]) & (1 << (7 - x))) {
          display.drawPixel(obstacleX + x, birdY + y, SSD1306_WHITE);
        }
      }
    }
  }

  // Suelo
  display.fillRect(0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT, SSD1306_WHITE);

  // Puntos
  display.setCursor(SCREEN_WIDTH - 30, 0);
  display.print(score);
}

void displayIntro() {
  display.clearDisplay();
  display.setCursor(10, 35);
  display.print("Dino Game");
  display.setCursor(10, 43);
  display.print("Press to start");
  display.display();
  while (analogRead(JUMP_BUTTON) > 100);
  delay(500);
}

void gameOver() {
  display.clearDisplay();
  display.setCursor(10, 35);
  // Sonido al perder
  playTone(130, 100);
  delay(50);
  playTone(130, 100);
  display.print("Game Over");
  display.setCursor(10, 43);
  display.print("Score: ");
  display.print(score);
  display.display();
  while (analogRead(JUMP_BUTTON) > 100);
  delay(1000);
  score = 0;
  obstacleX = SCREEN_WIDTH;
  displayIntro();
}

void playTone(int f, int d) {
  tone(BUZZ, f, d);
  delay(d);
  noTone(BUZZ);
}