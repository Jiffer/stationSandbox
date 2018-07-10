//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Listening station
// KGNU 40th anniversary at the Museum of Boulder
//
// by Jiffer Harriman
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioPlaySdWav           playWav1;
// Use one of these 3 output types: Digital I2S, Digital S/PDIF, or Analog DAC
AudioOutputI2S           audioOutput;
//AudioOutputSPDIF       audioOutput;
//AudioOutputAnalog      audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////
const int numButtons = 8;
int buttonPins[numButtons] = {32, 30, 28, 28, 28, 28, 28, 28};
int ledPins[numButtons] = {31, 29, 27, 27, 27, 27, 27, 27};

File root;

char fileNameList[100][16]; // currently a max of 100 tracks
char dirNameList[100][16];
char fList[numButtons][100][32]; // file location including directory name and filename.wav
int numTracks[numButtons] = {0, 0, 0, 0, 0, 0, 0, 0};
int currentTrack[numButtons] = {0, 0, 0, 0, 0, 0, 0, 0};
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Setup
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);

  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  root = SD.open("/");

  readDirectory(root, 0, -1);

  Serial.println("all done reading directories!");
  for (int i = 0; i < 8; i++) {
    Serial.print("tracks in folder ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(numTracks[i]);
  }

  // configure pins
  for (int i = 0; i < 8; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH);
  }
}



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Main loop
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void loop() {
  int buttonPressed = -1;
  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(buttonPins[i]) == 0) {
      buttonPressed = i;
      digitalWrite(ledPins[i], LOW);
    }
  }
  if (buttonPressed != -1) {

    Serial.print("button : ");
    Serial.print(buttonPressed);
    Serial.print("playing track: ");
    Serial.println(currentTrack[buttonPressed]); // track number

    playFile(buttonPressed);

  }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// playFile()
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void playFile(int buttonPressed)
{
  const char *filename = fList[buttonPressed][currentTrack[buttonPressed]];
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);

  // A brief delay for the library read WAV info
  delay(5);

  // uncomment to lock out and wait for the file to finish playing.
  //  while (playWav1.isPlaying()) {
  //    // uncomment these lines if you audio shield
  //    // has the optional volume pot soldered
  //    //float vol = analogRead(15);
  //    //vol = vol / 1024;
  //    // sgtl5000_1.volume(vol);
  //  }

  // how long to wait until all lights are out
  delay(100);

  for (int i = 0; i < numButtons; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  delay(100);
  for(int i = 0; i < 2000; i ++){
    digitalWrite(ledPins[buttonPressed], HIGH);
    delay(1);
    digitalWrite(ledPins[buttonPressed], LOW);
    delay(1);
  }

  for (int i = 0; i < numButtons; i++) {
    digitalWrite(ledPins[i], HIGH);
  }
  currentTrack[buttonPressed]++;
  currentTrack[buttonPressed] %= numTracks[buttonPressed];
}



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// readDirectory()
//
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void readDirectory(File dir, int numTabs, int currentPassIn) {
  int currentFolder = currentPassIn;
  char* entryName;
  char* lastDirName;// = (char*)malloc(32*sizeof(char));  // unitialized perhaps because it may be called before entry.name()?
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      if (currentFolder != -1) {
        Serial.print("found " );
        Serial.print(numTracks[currentFolder]);
        Serial.print(" valid .wav files in currentFolder: ");
        Serial.println(currentFolder);
      }
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    entryName = entry.name();

    boolean _start = false;
    if (entryName[0] == '_') {
      _start = true;
    }
    if (!_start) {
      int startingAt = 0;
      for (int i = 0; i < 16; i++) {
        if (entryName[i] == '.') {
          startingAt = i;
        }
      }

      Serial.print(" cf is : ");
      Serial.println(currentFolder);
      if (currentFolder != -1) {
        if (startingAt > 0) {
          if (entryName[startingAt + 1] == 'W' && entryName[startingAt + 2] == 'A' && entryName[startingAt + 3] == 'V') {
            // put it in an array!
            for (int i = 0; i < 16; i++) {
              dirNameList[numTracks[currentFolder]][i] = lastDirName[i];
            }
            for (int i = 0; i < 16; i++) {
              fileNameList[numTracks[currentFolder]][i] = entryName[i];

            }

            char* dirName = strcat(strcat(dirNameList[numTracks[currentFolder]], "/"), fileNameList[numTracks[currentFolder]]);
            for (int i = 0; i < 32; i++) {
              fList[currentFolder][numTracks[currentFolder]][i] = dirName[i]; // todo! use folder name

            }

            Serial.print("stored file name: ");
            Serial.print(fileNameList[numTracks[currentFolder]]);
            Serial.print(" at loc: ");
            Serial.print(numTracks[currentFolder]);
            Serial.print(" whole ");
            Serial.println( fList[currentFolder][numTracks[currentFolder]] );
            numTracks[currentFolder]++;

          }
        }
      }
      Serial.print(entry.name());

      if (entry.isDirectory()) {
        Serial.println("/");

        lastDirName = entry.name();
        currentFolder = -1;
        if (lastDirName[0] == 'A' ) {
          Serial.println("current folder is 0");
          currentFolder = 0;
        } else if (lastDirName[0] == 'B' ) {
          Serial.println("current folder is 1");
          currentFolder = 1;
        } else if (lastDirName[0] == 'C' ) {
          Serial.println("current folder is 2");
          currentFolder = 2;
        } else if (lastDirName[0] == 'D' ) {
          Serial.println("current folder is 3");
          currentFolder = 3;
        } else if (lastDirName[0] == 'E' ) {
          currentFolder = 4;
        } else if (lastDirName[0] == 'F' ) {
          currentFolder = 5;
        } else if (lastDirName[0] == 'G' ) {
          currentFolder = 6;
        } else if (lastDirName[0] == 'H' ) {
          currentFolder = 7;
        }

        // call back into itself
        readDirectory(entry, numTabs + 1, currentFolder);
      } else {
        // files have sizes, directories do not
        Serial.print("\t\t");
        Serial.println(entry.size(), DEC);
      }
    }
    entry.close();
  }
}
