#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RotaryEncoder.h>
#include <MIDI.h> // Standard hardware MIDI library

// --- Hardware MIDI Setup (UART0 / GP0 TX) ---
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// --- OLED Setup (SSD1306) ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Pin Definitions ---
#define ENC_PIN_A 2
#define ENC_PIN_B 3
#define BTN_ROUTING 14
#define POT1_PIN 26 // Reverb Decay
#define POT2_PIN 27 // Reverb Tweak
#define POT3_PIN 28 // Reverb Mix
#define POT4_PIN 29 // Delay Time

RotaryEncoder encoder(ENC_PIN_A, ENC_PIN_B, RotaryEncoder::LatchMode::FOUR3);

// --- Global Variables ---
int oldPosition = 0;
int currentReverb = 0;
int routingState = 0; 
bool lastBtnState = HIGH;
unsigned long lastBtnPressTime = 0; 

// Potentiometer storage
int lastPotValues[4] = {-1, -1, -1, -1};
int lastRawValues[4] = {0, 0, 0, 0};
const int potPins[4] = {POT1_PIN, POT2_PIN, POT3_PIN, POT4_PIN};
const int potCCs[4]  = {17, 18, 20, 11}; // CC Map: Decay, Tweak, Mix, Delay Time

// --- HUD (Heads-Up Display) variables ---
unsigned long lastPotInteractionTime = 0; 
const unsigned long HUD_TIMEOUT = 1500;   // Return to main screen after 1.5s
bool hudActive = false;                   
int activePotIndex = -1;                  
int activePotValue = 0;                   

// --- Database ---
const char* reverbNames[16] = {
  "Room", "Searchlights", "Particle Verb", "Double Tank",
  "Octo", "Tile", "Ducking", "Plateaux",
  "Cave", "Plate", "Ganymede", "Chamber",
  "Hot Springs", "Hall", "Glitz", "Reverb Off"
};

// Tweak parameter names per Reverb model (Based on DL4 MkII Cheat Sheet)
const char* tweakParamNames[16] = {
  "PreDelay",      // 0: Room
  "Mod Max Depth", // 1: Searchlights
  "Condition",     // 2: Particle Verb
  "Mod Depth",     // 3: Double Tank
  "Intensity",     // 4: Octo
  "PreDelay",      // 5: Tile
  "PreDelay",      // 6: Ducking
  "Pitch Mode",    // 7: Plateaux
  "PreDelay",      // 8: Cave
  "PreDelay",      // 9: Plate
  "Mod Depth",     // 10: Ganymede
  "PreDelay",      // 11: Chamber
  "Spring Count",  // 12: Hot Springs
  "PreDelay",      // 13: Hall
  "Mod Depth",     // 14: Glitz
  "-"              // 15: Reverb Off
};

const char* routingNames[3] = { "Pre (Before Delay)", "Parallel", "Post (After Delay)" };

void setup() {
  // Initialize Hardware MIDI
  Serial1.setTX(0); // Assign TX pin to GP0 (Pin 1)
  MIDI.begin(MIDI_CHANNEL_OMNI);

  pinMode(BTN_ROUTING, INPUT_PULLUP);

  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();

  // Initialize SSD1306 (Typical address: 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Fallback to 0x3D
      for(;;); 
    }
  }

  display.clearDisplay();
  updateOLED();
}

void loop() {
  bool needsUpdate = false;

  // 1. Rotary Encoder Handling (CC#2)
  encoder.tick();
  int newPos = encoder.getPosition();
  if (newPos != oldPosition) {
    if (newPos > oldPosition) currentReverb--;
    else currentReverb++;

    // Loop 0-15
    if (currentReverb > 15) currentReverb = 0;
    if (currentReverb < 0) currentReverb = 15;
    
    oldPosition = newPos;
    hudActive = false; // Cancel HUD display when switching models
    needsUpdate = true; 
    
    MIDI.sendControlChange(2, currentReverb, 1);
  }

  // 2. Button Handling (CC#19 Routing)
  bool btnState = digitalRead(BTN_ROUTING);
  if (lastBtnState == HIGH && btnState == LOW && (millis() - lastBtnPressTime > 200)) {
    routingState++;
    if (routingState > 2) routingState = 0;
    
    hudActive = false; // Cancel HUD display when toggling routing
    needsUpdate = true; 
    lastBtnPressTime = millis();
    
    MIDI.sendControlChange(19, routingState, 1);
  }
  lastBtnState = btnState;

  // 3. Potentiometer Handling
  for (int i = 0; i < 4; i++) {
    if (readPot(i)) {
      needsUpdate = true;
    }
  }

  // 4. HUD Timeout Logic
  if (hudActive && (millis() - lastPotInteractionTime > HUD_TIMEOUT)) {
    hudActive = false;
    needsUpdate = true; 
  }

  // 5. Update OLED
  if (needsUpdate) {
    updateOLED();
  }
  delay(5);
}

// Function to read Pots, send MIDI, and trigger HUD
bool readPot(int index) {
  int rawSum = 0;
  for (int j = 0; j < 8; j++) {
    rawSum += analogRead(potPins[index]);
  }
  int rawValue = rawSum / 8;
  int midiValue = rawValue >> 3; // Convert 10-bit (0-1023) to 7-bit MIDI (0-127)
  
  if (midiValue != lastPotValues[index]) {
    // Threshold set to 8 for smooth 1-step increments while preventing jitter
    if (abs(rawValue - lastRawValues[index]) >= 8 || midiValue == 0 || midiValue == 127) { 
      MIDI.sendControlChange(potCCs[index], midiValue, 1);
      
      lastPotValues[index] = midiValue;
      lastRawValues[index] = rawValue;

      // Trigger HUD display
      lastPotInteractionTime = millis();
      hudActive = true;
      activePotIndex = index;
      activePotValue = midiValue;

      return true;
    }
  }
  return false;
}

// UI Display Function
void updateOLED() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE); 

  if (hudActive && activePotIndex >= 0) {
    // ==========================================
    //  HUD View: Shown during knob adjustment
    // ==========================================
    
    // Line 1: Current Reverb model name
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("[ "));
    display.print(reverbNames[currentReverb]);
    display.println(F(" ]"));
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE); // Divider line

    // Line 2: Active parameter name
    display.setCursor(0, 16);
    if (activePotIndex == 0)      display.println(F("Reverb Decay"));
    else if (activePotIndex == 1) display.println(tweakParamNames[currentReverb]); 
    else if (activePotIndex == 2) display.println(F("Reverb Mix"));
    else if (activePotIndex == 3) display.println(F("Delay Time"));

    // Line 3: Current value
    display.setTextSize(2);
    display.setCursor(0, 32);
    
    if (activePotIndex == 2) {
      // --- Dry/Wet ratio for Reverb Mix ---
      int wetPercent = map(activePotValue, 0, 127, 0, 100);
      int dryPercent = 100 - wetPercent;
      
      display.print(F("D"));
      display.print(dryPercent);
      display.print(F(":W"));
      display.println(wetPercent);
    } else {
      // --- Standard numeric value for other parameters ---
      display.println(activePotValue);
    }

    // Progress bar at the bottom
    display.drawRect(0, 54, 128, 6, SSD1306_WHITE); 
    int barWidth = map(activePotValue, 0, 127, 0, 124);
    display.fillRect(2, 56, barWidth, 2, SSD1306_WHITE);

    // Draw center indicator line for Reverb Mix (50% mark)
    if (activePotIndex == 2) {
      display.drawLine(64, 53, 64, 60, SSD1306_INVERSE); 
    }

  } else {
    // ==========================================
    //  Idle View: Default screen
    // ==========================================
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("--- DL4 MkII CTRL ---"));
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    display.setCursor(0, 24);
    display.print(F("Model: "));
    display.println(reverbNames[currentReverb]);

    display.setCursor(0, 44);
    display.print(F("Route: "));
    display.println(routingNames[routingState]);
  }

  display.display();
}