#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Replace with your network credentials
const char* ssid = "ammulu";
const char* password = "2008A2008##";

// Define the NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -5 * 3600, 60000); // UTC-6 for Central Time (Standard Time)

// OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_CS     14
#define OLED_DC     27
#define OLED_RESET_PIN  26
#define OLED_MOSI   25
#define OLED_CLK    13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET_PIN, OLED_CS);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize NTPClient
  timeClient.begin();
}

String formatTime(int hours, int minutes, int seconds) {
  String period = " AM";
  if (hours >= 12) {
    period = " PM";
    if (hours > 12) {
      hours -= 12;
    }
  }
  if (hours == 0) {
    hours = 12;
  }

  char timeString[11];
  sprintf(timeString, "%02d:%02d:%02d%s", hours, minutes, seconds, period.c_str());
  return String(timeString);
}

void loop() {
  timeClient.update();

  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  int seconds = timeClient.getSeconds();

  String formattedTime = formatTime(hours, minutes, seconds);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 22);
  display.print(formattedTime);

  display.display();

  delay(1000);
}