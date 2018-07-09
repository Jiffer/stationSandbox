
//
//   1: Digital I2S - Normally used with the audio shield:
//         http://www.pjrc.com/store/teensy3_audio.html
//


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

int buttonPin = 32;
boolean okToPlay = false;
File root;

char fileNameList[100][16]; // sets a max of 100 tracks
char dirNameList[100][16];
char fList[8][100][32]; // file location including directory name and filename.wav
int numTracks[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int currentTrack[8] = {0, 0, 0, 0, 0, 0, 0, 0};
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

  Serial.println("all done with setup!");

  // configure pins
  pinMode(buttonPin, INPUT_PULLUP);
}

void playFile(const char *filename)
{
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

  // how long to wait until buttons become active again:
  delay(1000);
}


void loop() {

  if (digitalRead(buttonPin) == 0) {
    int folder = 0;
    Serial.print("playing track: ");
    Serial.println(currentTrack[folder]); // track number

    playFile(fList[folder][currentTrack[folder]]);
    currentTrack[folder]++;
    currentTrack[folder] %= numTracks[folder];
    delay(500);
  }
}

void readDirectory(File dir, int numTabs, int currentPassIn) {
  int currentFolder = currentPassIn;
  char* entryName;
  char* lastDirName;// = (char*)malloc(32*sizeof(char));
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      if (currentFolder != -1) {
        Serial.print("found this many valid .wav files: ");
        Serial.println(numTracks[currentFolder]);
      }
      //      free(lastDirName);
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
      if (startingAt > 0) {
        if (entryName[startingAt + 1] == 'W' && entryName[startingAt + 2] == 'A' && entryName[startingAt + 3] == 'V') {
          //          char* dirName;
          // put it in an array!
          for (int i = 0; i < 16; i++) {
            dirNameList[numTracks[currentFolder]][i] = lastDirName[i];
          }
          for (int i = 0; i < 16; i++) {
            fileNameList[numTracks[currentFolder]][i] = entryName[i];

          }



          Serial.print(" cf is : ");
          Serial.println(currentFolder);
          if (currentFolder != -1) {
            Serial.println("current folder is aewersomes");
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


        Serial.println("/");
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
