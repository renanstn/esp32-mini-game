#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 8
#define PLAYER_Y_POSITION 50
#define BUZZER_PIN 18

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
int score = 0;
unsigned long last_millis = 0;

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

  // Setup buzzer
  //ledcSetup(0, 5000, 10);
  //ledcAttachPin(BUZZER_PIN, 0);
  //ledcWriteTone(0, 5000);
}


void loop()
{
  // Read input
  potentiometer_read = analogRead(A0);

  // Calc player position
  player_position = (int)map(potentiometer_read, 4095, 0, 0, SCREEN_WIDTH - PLAYER_WIDTH - 1);
  // Limita a posição do player dentro da pista
  player_position = constrain(player_position, 10, display.width()-1-PLAYER_WIDTH-10);
  display.clearDisplay();

  // Draw player
  // display.drawPixel(player_position, 63, WHITE);
  display.drawBitmap(player_position, PLAYER_Y_POSITION, player, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);

  // Draw extern road line
  display.drawLine(30, 16, 0, display.height()-1, WHITE);
  display.drawLine(display.width()-30-1, 16, display.width()-1, display.height()-1, WHITE);
  // Draw intern road line
  display.drawLine(31, 16, 0, display.height()-1+6, WHITE);
  display.drawLine(display.width()-31-1, 16, display.width()-1, display.height()-1+6, WHITE);

  // Draw Score
  display.setCursor(0, 0);
  display.print("Score: 000");

  // Render the buffer
  display.display();
  delay(15);
}
