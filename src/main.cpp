#include <Arduino.h>     // Arduino framework
#include <SPIFFS.h>      // Library for SPIFFS file system
#include <driver/i2s.h>  // Library for I2S audio processing
#include <Audio.h>       // Library for audio processing
#include <Preferences.h> // Library for saving data in the flash memory
#include <OneButton.h>

// Define the pins for I2S for audio amplifier(MAX98357A)
#define MAX98357A_DIN_PIN 26  // Data in pin to data input
#define MAX98357A_BCLK_PIN 27 // clock pin of the i2s audio amplifier
#define MAX98357A_LRC_PIN 14  // LRC pin of the 12s audio amplifier

// Define pin for potentiometer input
#define pausePot_Pin 15

// Define audio class
Audio audio;
int audio_volume = 21; // 0-21
int audioPauseTime;    // 2 seconds - 30 seconds

Preferences preferences;

// Define pin for switch for play/ pause and sleep control
#define Button_Pin 4

// Onebutton object for the button
OneButton button(Button_Pin, true); // button enabled for active low(true)
unsigned long longPressStartTime = 0;
// Flag to check if the audio is paused or not
bool isPaused = false;
// Flag to check if the button is long pressed
bool isLongPress = false;

// this function handles the play/pause state of audio
void handleClick()
{
  if (isPaused)
  {
    isPaused = false;
    preferences.putBool("isPaused", false);
    Serial.println("Audio resumed!");
    audio.pauseResume();
    audio.connecttoFS(SPIFFS, "/NAB_Notice.wav");
  }
  else
  {
    isPaused = true;
    preferences.putBool("isPaused", true);
    Serial.println("Audio paused!");
    audio.pauseResume();
  }
}

// this function handles the long press of the button for esp deepsleep
void handleLongPressStart()
{
  Serial.println("Going to deep sleep...");
  // digitalWrite(Button_Pin , HIGH);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)Button_Pin, 0); // Wake up on low level
  esp_deep_sleep_start();
}

void setup()
{
  Serial.begin(115200); // Initialize the serial port

  // Setting up the pot pinMode as input
  pinMode(pausePot_Pin, INPUT);

  // Setting up the button
  button.attachClick(handleClick);
  // button.attachDuringLongPress(handleLongPressStart);
  button.attachLongPressStop(handleLongPressStart);
  button.setPressMs(4000); // 5 seconds button hold duration.

  // Initialize the SPIFFS file system
  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
  delay(100);

  // Preferences to save the pause_time
  preferences.begin("AUDIO_CONFIG", false);
  audioPauseTime = preferences.getInt("pause_time", 15);
  isPaused = preferences.getBool("isPaused", false);
  // preferences.end();

  // Initialize the I2S audio processing
  audio.setPinout(MAX98357A_BCLK_PIN, MAX98357A_LRC_PIN, MAX98357A_DIN_PIN);
  audio.setVolume(audio_volume);
  // audio.connecttoFS(SPIFFS, "/NAB_Notice.wav");
}

void loop()
{
  audio.loop();

  int pause_pot_value = analogRead(pausePot_Pin);        // Read potentiometer value for pause timer
  audioPauseTime = map(pause_pot_value, 0, 4095, 2, 30); // Constraining the values to the desired range

  // save the pause time to preference
  //  preferences.begin("AUDIO_CONFIG");
  preferences.putInt("pause_time", audioPauseTime);
  // preferences.end();

  // Printing the value of pause time
  Serial.print("Pause time: ");
  Serial.print(audioPauseTime);
  Serial.print(" seconds.");

  button.tick(); // updating the button state

  // variable to

  // Check the isPaused state to play the audio
  if (!isPaused)
  {
    if (!audio.isRunning())
    {
      delay(audioPauseTime * 1000);
      audio.connecttoFS(SPIFFS, "/NAB_Notice.wav");
    }
    else
    {
      Serial.println("Audio is playing.");
    }
  }
  else
  {
    Serial.println("Audio is not playing.");
  }
  // Check if the audio has finished playing
  // if (!audio.isRunning()) {
  //   // Restart the audio
  //   delay(audioPauseTime*1000);
  //   audio.connecttoFS(SPIFFS, "/NAB_Notice.wav");
  // }
  // delay(2000); // Adjust delay as needed
}
