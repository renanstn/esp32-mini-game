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

struct RoadDetail {
  int x;
  int y;
};

float potentiometer_read;
int player_position;
int score = 0;
RoadDetail left_road_details[3];
RoadDetail right_road_details[3];
unsigned long last_millis = 0;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


int calc_x_position_of_road_detail(int y_position, char side) {
  int result;
  if (side == 'l') {
    result = map(y_position, 16, SCREEN_HEIGHT, 36, 0+6);
  } else if (side == 'r') {
    result = map(y_position, 16, SCREEN_HEIGHT, SCREEN_WIDTH-1-36, SCREEN_WIDTH-1-6);
  }
  return result;
}


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
//  ledcSetup(0, 5000, 10);
//  ledcAttachPin(BUZZER_PIN, 0);
//  ledcWriteTone(0, 4000);

  // Setup initial position of road details
  left_road_details[0] = RoadDetail{calc_x_position_of_road_detail(16, 'l'), 16};
  left_road_details[1] = RoadDetail{calc_x_position_of_road_detail(32, 'l'), 32};
  left_road_details[2] = RoadDetail{calc_x_position_of_road_detail(48, 'l'), 48};
  right_road_details[0] = RoadDetail{calc_x_position_of_road_detail(16, 'r'), 16};
  right_road_details[1] = RoadDetail{calc_x_position_of_road_detail(32, 'r'), 32};
  right_road_details[2] = RoadDetail{calc_x_position_of_road_detail(48, 'r'), 48};
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
  // Draw road details
  for (int i=0; i<3; i++) {
    display.drawPixel(left_road_details[i].x, left_road_details[i].y, WHITE);
    display.drawPixel(right_road_details[i].x, right_road_details[i].y, WHITE);
  }
  // Animate road details
  for (int i=0; i<3; i++) {
    left_road_details[i].y += 2;
    left_road_details[i].x = calc_x_position_of_road_detail(left_road_details[i].y, 'l');
    right_road_details[i].y += 2;
    right_road_details[i].x = calc_x_position_of_road_detail(right_road_details[i].y, 'r');
    // Return details to top screen when 
    if (left_road_details[i].y > SCREEN_HEIGHT) {
      left_road_details[i].y = 16;
      left_road_details[i].x = calc_x_position_of_road_detail(left_road_details[i].y, 'l');
      right_road_details[i].y = 16;
      right_road_details[i].x = calc_x_position_of_road_detail(right_road_details[i].y, 'r');
    }
  }

  // Draw Score
  display.setCursor(0, 0);
  display.print("Score: 000");

  // Render the buffer
  display.display();
  delay(15);
}
