#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int JUMP_BUTTON = A2;

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

const int DINO_WIDTH = 9;
const int DINO_HEIGHT = 15;
const int GROUND_HEIGHT = 5;
const int OBSTACLE_WIDTH = 3;
const int OBSTACLE_HEIGHT = 8;

int dinoY;
int obstacleX;
bool isJumping = false;
int jumpPhase = 0;
const int JUMP_HEIGHT = 10;
const int JUMP_DURATION = 6;
unsigned long score = 0;

void setup() {
  pinMode(JUMP_BUTTON, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;
  obstacleX = SCREEN_WIDTH;

  displayIntro();
}

void loop() {
  if (analogRead(JUMP_BUTTON) < 100 && !isJumping) {
    isJumping = true;
    jumpPhase = 0;
  }

  updateGame();
  drawGame();
  
  display.display();
  delay(50);  // Control game speed
  score++;
}

void updateGame() {
  if (isJumping) {
    if (jumpPhase < JUMP_DURATION) {
      // Rising
      dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT - (JUMP_HEIGHT * jumpPhase / JUMP_DURATION);
    } else if (jumpPhase < JUMP_DURATION * 2) {
      // Holding
      dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT - JUMP_HEIGHT;
    } else if (jumpPhase < JUMP_DURATION * 3) {
      // Falling
      dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT - (JUMP_HEIGHT * (JUMP_DURATION * 3 - jumpPhase) / JUMP_DURATION);
    } else {
      // Jump complete
      isJumping = false;
      dinoY = SCREEN_HEIGHT - DINO_HEIGHT - GROUND_HEIGHT;
    }
    jumpPhase++;
  }

  obstacleX -= 2;
  if (obstacleX < -OBSTACLE_WIDTH) {
    obstacleX = SCREEN_WIDTH;
  }

  // Check for collision
  if (obstacleX < DINO_WIDTH && 
      obstacleX + OBSTACLE_WIDTH > 0 && 
      dinoY + DINO_HEIGHT > SCREEN_HEIGHT - GROUND_HEIGHT - OBSTACLE_HEIGHT) {
    gameOver();
  }
}

void drawGame() {
  display.clearDisplay();

  // Draw dino
  for (int y = 0; y < DINO_HEIGHT; y++) {
    for (int x = 0; x < DINO_WIDTH; x++) {
      if (pgm_read_byte(&DINO_BITMAP[y]) & (1 << (7-x))) {
        display.drawPixel(x, dinoY + y, SSD1306_WHITE);
      }
    }
  }

  // Draw obstacle
  display.fillRect(obstacleX, SCREEN_HEIGHT - GROUND_HEIGHT - OBSTACLE_HEIGHT, OBSTACLE_WIDTH, OBSTACLE_HEIGHT, SSD1306_WHITE);

  // Draw ground
  display.fillRect(0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT, SSD1306_WHITE);

  // Draw score
  display.setCursor(SCREEN_WIDTH - 30, 0);
  display.print(score);
}

void displayIntro() {
  display.clearDisplay();
  display.setCursor(10, 10);
  display.print("Dino Game");
  display.setCursor(10, 20);
  display.print("Press to start");
  display.display();
  while(analogRead(JUMP_BUTTON) > 100);
  delay(500);
}

void gameOver() {
  display.clearDisplay();
  display.setCursor(10, 10);
  display.print("Game Over");
  display.setCursor(10, 20);
  display.print("Score: ");
  display.print(score);
  display.display();
  while(analogRead(JUMP_BUTTON) > 100);
  delay(1000);
  score = 0;
  obstacleX = SCREEN_WIDTH;
  displayIntro();
}
