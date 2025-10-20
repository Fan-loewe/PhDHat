// ------------------ IMPORTS ------------------
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <U8g2lib.h>
#include <SPI.h>

// ------------------ PINS ------------------
#define NEOPIXEL_PIN 6
#define NUMPIXELS 12
#define DFPLAYER_RX 4
#define DFPLAYER_TX 5
#define DFPLAYER_BUSY 7
#define SSD1322_CS 10
#define SSD1322_DC 9
#define SSD1322_RESET 8
#define MPU_ADDR 0x68
#define MQ3_PIN A0


// ------------------ OBJECTS ------------------
Adafruit_NeoPixel ring(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial mySerial(DFPLAYER_RX, DFPLAYER_TX); 
DFRobotDFPlayerMini myDFPlayer;
U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI u8g2(U8G2_R0, SSD1322_CS, SSD1322_DC, SSD1322_RESET);


// --------------- PARAMETERS --------------
  // -------- IMU ---------
float smoothedAx=0, smoothedAy=0, smoothedAz=0;
int16_t lastAx=0, lastAy=0, lastAz=0;
const long SHAKE_THRESHOLD = 5000000;
const float alpha_imu = 0.5;
  // --------- CELEBRATION LOGIC ---------
enum AppMode { IDLE_MODE, CELEBRATION_MODE, DRUNK_MODE } 
appMode = IDLE_MODE;
enum SubStep { INTRO_CELEBRATION_MODE, WAIT_SENSOR_CELEBRATION, SENSOR_TRIGGERED, SENSOR_NOT_TRIGGERED, PLAY_FINAL_AUDIO, END_SUBSTEP };
SubStep currentSubStep = INTRO_CELEBRATION_MODE;
enum DrunkStep { INTRO_DRUNK, WAIT_SENSOR_DRUNK, DRUNK_SENSOR_TRIGGERED, DRUNK_SENSOR_NOT_TRIGGERED, DRUNK_PLAY_FINAL_AUDIO, DRUNK_END_SUBSTEP};
DrunkStep currentDrunkStep = INTRO_DRUNK;

  // --------- VARIABLES ---------
unsigned long subStepStartTime = 0;
int reminderCount = 0;
const int MAX_REMINDERS = 1;
const unsigned long REMINDER_INTERVAL = 5000; // 5 sec between reminders
int rainbowJ = 0;

  // --------- LED ---------
unsigned long lastLedUpdate = 0;
int theaterChaseStep = 0;

  // ------------------ MQ3 Sensor ------------------
float avgValue = 0;
const float alpha_mq3 = 0.1;   // smoothing factor
const int DRUNK_THRESHOLD = 300;
bool alcoholDetected = false;


// ------------------ METHODS ------------------
  // --------- SETUP ---------
void setup_serial_connection_computer() {
  Serial.begin(9600);
}
void setup_pixel_led_ring() {
  ring.begin();
  ring.show();
  for (int i = 0; i < NUMPIXELS; i++)
    ring.setPixelColor(i, ring.Color(50,50,50)); // moderate brightness
  ring.show();
}
void setup_df_player(){
  mySerial.begin(9600);
  Serial.println("Initializing DFPlayer Mini...");
  if (!myDFPlayer.begin(mySerial)) { 
    Serial.println("Unable to begin DFPlayer Mini!"); 
    while(true); 
  }
  myDFPlayer.volume(20);
}
void setup_oled_screen(){
  u8g2.begin();
  u8g2.clearBuffer();
}

void setup_mq3(){
  pinMode(MQ3_PIN, INPUT);
}

void setup_imu(){
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  imu_read_acceleration(lastAx,lastAy,lastAz);
  smoothedAx=lastAx; smoothedAy=lastAy; smoothedAz=lastAz;
}
  // --------- MUSIC FUNCTIONS ---------
void play_music_track(int track, bool wait_for_finish){
    myDFPlayer.play(track);
    Serial.print("Playing track "); Serial.println(track);
    delay(300);
    if (wait_for_finish) {
      wait_music_to_finish();
    }
}
void wait_music_to_finish(){
    while (digitalRead(DFPLAYER_BUSY) == LOW) {
      delay(100);
  }
}

void introduction_music(){
  for (int track = 1; track <= 8; track++) {
    play_music_track(track, true);
    delay(300);
  }
}
void introduction_bodyactivity_measurement(){
  play_music_track(9, true);
  play_music_track(10, true);
}

void introduction_alcohol(){
  play_music_track(16, true);
  play_music_track(17, true);
}

void end_music(){
  for (int track = 24; track <= 26; track++) {
    play_music_track(track, true);
    delay(300);
  }
}

  // --------- DISPLAY FUNCTIONS ---------
void display_screen_message(String msg) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(5,30,msg.c_str());
  } while(u8g2.nextPage());
}

// --------- PIXEL LED RING FUNCTIONS ---------
void run_green_led_animation() {
  unsigned long now = millis();
  if(now - lastLedUpdate > 50) {
    ring.clear();
    for(int c=theaterChaseStep; c<ring.numPixels(); c+=3)
      ring.setPixelColor(c, ring.Color(0,255,0));
    ring.show();
    theaterChaseStep = (theaterChaseStep+1)%3;
    lastLedUpdate = now;
  }
}
void run_red_led_animation() {
  unsigned long now = millis();
  if(now - lastLedUpdate > 50) {
    ring.clear();
    for(int c=theaterChaseStep; c<ring.numPixels(); c+=3)
      ring.setPixelColor(c, ring.Color(255,0,0));
    ring.show();
    theaterChaseStep = (theaterChaseStep+1)%3;
    lastLedUpdate = now;
  }
}

void run_blue_led_animation() {
  unsigned long now = millis();
  if(now - lastLedUpdate > 50) {
    ring.clear();
    for(int c=theaterChaseStep; c<ring.numPixels(); c+=3)
      ring.setPixelColor(c, ring.Color(0,0,255));
    ring.show();
    theaterChaseStep = (theaterChaseStep+1)%3;
    lastLedUpdate = now;
  }
}

void run_amber_yellow_led() {
  unsigned long now = millis();
  if (now - lastLedUpdate > 100) {
    ring.clear();
    for (int c = theaterChaseStep; c < ring.numPixels(); c += 3)
      ring.setPixelColor(c, ring.Color(255, 150, 0)); // orange/yellow
    ring.show();
    theaterChaseStep = (theaterChaseStep + 1) % 3;
    lastLedUpdate = now;
  }
}

void run_solid_red_led() {
  ring.clear();
  for (int i = 0; i < ring.numPixels(); i++)
    ring.setPixelColor(i, ring.Color(255, 0, 0));
  ring.show();
}


void run_fancy_led() {
  unsigned long now = millis();
  if (now - lastLedUpdate > 40) {
    for (int i = 0; i < ring.numPixels(); i++) {
      int pixelHue = (i * 256 / ring.numPixels() + rainbowJ) & 255;
      ring.setPixelColor(i, Wheel(pixelHue));
    }
    ring.show();
    rainbowJ = (rainbowJ + 2) % 256;
    lastLedUpdate = now;
  }
}

// ------------------ COLOR WHEEL ------------------
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) return ring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  if(WheelPos < 170) { WheelPos -= 85; return ring.Color(0, WheelPos * 3, 255 - WheelPos * 3); }
  WheelPos -= 170;
  return ring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}



  // --------- IMU FUNCTIONS ---------
float imu_detect_shake() {
  int16_t ax, ay, az;
  imu_read_acceleration(ax, ay, az);
  smoothedAx = alpha_imu*ax + (1-alpha_imu)*smoothedAx;
  smoothedAy = alpha_imu*ay + (1-alpha_imu)*smoothedAy;
  smoothedAz = alpha_imu*az + (1-alpha_imu)*smoothedAz;
  float dAx = smoothedAx - lastAx;
  float dAy = smoothedAy - lastAy;
  float dAz = smoothedAz - lastAz;
  lastAx = smoothedAx; lastAy = smoothedAy; lastAz = smoothedAz;
  float deltaMagSq = dAx*dAx + dAy*dAy + dAz*dAz;
  return deltaMagSq;
}
void imu_read_acceleration(int16_t &ax,int16_t &ay,int16_t &az) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR,6,true);
  ax = Wire.read()<<8 | Wire.read();
  ay = Wire.read()<<8 | Wire.read();
  az = Wire.read()<<8 | Wire.read();
}



// ------------------ SETUP ------------------
void setup() {
  setup_serial_connection_computer();
  setup_pixel_led_ring();
  setup_df_player();
  setup_oled_screen();
  setup_imu();
  setup_mq3();
  display_screen_message("Welcome Patrick Langer");
  introduction_music();

  // Switch to celebration mode after initial audio
  appMode = CELEBRATION_MODE;
  currentSubStep = INTRO_CELEBRATION_MODE;
}


// ------------------ MAIN LOOP ------------------
void loop() {
  unsigned long now = millis();

  if(appMode == CELEBRATION_MODE) {
    switch(currentSubStep) {

      case INTRO_CELEBRATION_MODE:
        display_screen_message("Step 1. Human Activity Detection");

        introduction_bodyactivity_measurement();
        subStepStartTime = now;
        reminderCount = 0;
        currentSubStep = WAIT_SENSOR_CELEBRATION;
        run_blue_led_animation();
        play_music_track(11, false);
        break;

      case WAIT_SENSOR_CELEBRATION: {
        while (digitalRead(DFPLAYER_BUSY) == LOW) {
          float deltaMagSq = imu_detect_shake();
          Serial.print("DeltaMagSq: "); Serial.println(deltaMagSq);
          if(deltaMagSq > SHAKE_THRESHOLD) {
            currentSubStep = SENSOR_TRIGGERED;
            break;
          }
        }
        if (currentSubStep!=SENSOR_TRIGGERED){
          currentSubStep = SENSOR_NOT_TRIGGERED;
        }
        break;
      }

      case SENSOR_NOT_TRIGGERED:
        display_screen_message("Body activity not detected");
        run_red_led_animation();
        play_music_track(12, true);
        play_music_track(13, true);
        play_music_track(15, true);
        currentSubStep = PLAY_FINAL_AUDIO; 
        break;

      case SENSOR_TRIGGERED:
        display_screen_message("Body activity detected");
        run_green_led_animation();
        play_music_track(12, true);
        play_music_track(13, true);
        play_music_track(14, true);
        currentSubStep = PLAY_FINAL_AUDIO; // go play audio 12–14
        break;

      case PLAY_FINAL_AUDIO:
        currentSubStep = END_SUBSTEP;
        break;

      case END_SUBSTEP:
        run_amber_yellow_led();
        appMode = DRUNK_MODE;
        break;
    }
  }

  if(appMode == DRUNK_MODE) {
    switch(currentDrunkStep) {

      case INTRO_DRUNK:
        display_screen_message("Step 2. Alcohol Detection");

        introduction_alcohol();
        subStepStartTime = now;
        reminderCount = 0;
        currentDrunkStep = WAIT_SENSOR_DRUNK;
        run_amber_yellow_led();
        play_music_track(18, false);
        break;

      case WAIT_SENSOR_DRUNK: {
        while (digitalRead(DFPLAYER_BUSY) == LOW) {
          // Read MQ-3 and smooth
          int rawValue = analogRead(MQ3_PIN);
          avgValue = (1 - alpha_mq3) * avgValue + alpha_mq3 * rawValue;
          Serial.print("Raw: "); Serial.print(rawValue);
          Serial.print(" | Smoothed: "); Serial.println(avgValue);

          // Check threshold
          if (avgValue > DRUNK_THRESHOLD) {
            alcoholDetected = true;
            currentDrunkStep = DRUNK_SENSOR_TRIGGERED;
            break;
          }
        }
        if (currentDrunkStep!=DRUNK_SENSOR_TRIGGERED){
          currentDrunkStep = DRUNK_SENSOR_NOT_TRIGGERED;
        }
        break;
      }

      case DRUNK_SENSOR_NOT_TRIGGERED:
        display_screen_message("Alcohol not detected");
        run_amber_yellow_led();
        play_music_track(20, true);
        play_music_track(22, true);
        currentDrunkStep = DRUNK_PLAY_FINAL_AUDIO; 
        break;

      case DRUNK_SENSOR_TRIGGERED:
        display_screen_message("Alcohol detected");
        run_solid_red_led();
        play_music_track(20, true);
        play_music_track(21, true);
        currentDrunkStep = DRUNK_PLAY_FINAL_AUDIO; 
        break;

      case DRUNK_PLAY_FINAL_AUDIO:
        play_music_track(23, true);
        currentDrunkStep = DRUNK_END_SUBSTEP;
        break;

      case DRUNK_END_SUBSTEP:
        display_screen_message("Time to Party!");
        run_fancy_led();
        end_music();
        break;
    }
  }
}

