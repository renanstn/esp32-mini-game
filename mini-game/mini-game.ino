#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 8
#define PLAYER_Y_POSITION 50

static const unsigned char PROGMEM player[] = {
  B00011000,
  B00011000,
  B00011000,
  B11111111,
  B00011000,
  B00011000,
  B00011000,
  B11111111,
};

float potentiometer_read;
int player_position;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() 
{
  Serial.begin(9600);
  
  // Setup display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.display();
  delay(1000);
  display.clearDisplay();
}

void loop() 
{
  // Read input
  potentiometer_read = analogRead(A0);
  
  // Calc player position
  player_position = (int)map(potentiometer_read, 4095, 0, 0, SCREEN_WIDTH - PLAYER_WIDTH - 1);
  display.clearDisplay();
  
  // Draw player
  // display.drawPixel(player_position, 63, WHITE);
  display.drawBitmap(player_position, PLAYER_Y_POSITION, player, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);

  // Draw circuit
  display.drawLine(30, 16, 0, display.height()-1, WHITE);
  display.drawLine(display.width()-30-1, 16, display.width()-1, display.height()-1, WHITE);

  // Draw Score
  display.setCursor(64, 0);
  display.print("Score: 000");
  
  // Render the buffer
  display.display();
  delay(15);
}
