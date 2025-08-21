#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED display SPI pins
#define OLED_MOSI  11
#define OLED_CLK   12
#define OLED_DC    9
#define OLED_CS    8
#define OLED_RESET 10

// Button pins
#define BUTTON_PIN 3 // Define as per your setup

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Snake properties
#define SNAKE_MAX_LENGTH 32
int snakeX[SNAKE_MAX_LENGTH];
int snakeY[SNAKE_MAX_LENGTH];
int snakeLength = 5;
int foodX, foodY;
int direction = 0; // 0 = right, 1 = down, 2 = left, 3 = up
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  // Initialize button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();

  // Initialize snake in the middle of the screen
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = SCREEN_WIDTH / 2 - i;
    snakeY[i] = SCREEN_HEIGHT / 2;
  }

  // Place initial food
  placeFood();
}

void loop() {
  // Read button state with debounce
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        direction = (direction + 1) % 4; // Change direction clockwise
      }
    }
  }
  lastButtonState = reading;

  // Move snake
  moveSnake();

  // Check for collisions
  if (checkCollision()) {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 30);
    display.print(F("Game Over"));
    display.display();
    while (true); // Stop the game
  }

  // Check if food is eaten
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLength++;
    placeFood();
  }

  // Display the snake and food
  display.clearDisplay();
  for (int i = 0; i < snakeLength; i++) {
    display.drawRect(snakeX[i], snakeY[i], 2, 2, SSD1306_WHITE);
  }
  display.drawRect(foodX, foodY, 2, 2, SSD1306_WHITE);
  display.display();

  delay(200); // Adjust speed of the snake
}

void moveSnake() {
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  switch (direction) {
    case 0: snakeX[0]++; break; // Move right
    case 1: snakeY[0]++; break; // Move down
    case 2: snakeX[0]--; break; // Move left
    case 3: snakeY[0]--; break; // Move up
  }

  // Wrap around screen
  if (snakeX[0] >= SCREEN_WIDTH) snakeX[0] = 0;
  if (snakeX[0] < 0) snakeX[0] = SCREEN_WIDTH - 1;
  if (snakeY[0] >= SCREEN_HEIGHT) snakeY[0] = 0;
  if (snakeY[0] < 0) snakeY[0] = SCREEN_HEIGHT - 1;
}

void placeFood() {
  foodX = random(SCREEN_WIDTH / 2) * 2;
  foodY = random(SCREEN_HEIGHT / 2) * 2;
}

bool checkCollision() {
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      return true;
    }
  }
  return false;
}
