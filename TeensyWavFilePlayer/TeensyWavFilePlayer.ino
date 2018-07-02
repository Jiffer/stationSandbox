
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

char fileList[][13] = {"Bobby.wav", "Chicano.wav", "Solivan.wav", "StCanRan.wav", "YoLaTeng.wav"};
char fileNameList[100][16]; // max of 100 tracks
char dirNameList[100][16];
char fList[100][32];
int numTracks = 0;
int track = 0;
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

  printDirectory(root, 0);

  Serial.println("done!");
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

  // Simply wait for the file to finish playing.
//  while (playWav1.isPlaying()) {
    // uncomment these lines if you audio shield
    // has the optional volume pot soldered
    //float vol = analogRead(15);
    //vol = vol / 1024;
    // sgtl5000_1.volume(vol);
//  }
  
  delay(1000);
}


void loop() {

  if (digitalRead(buttonPin) == 0) {
    // concatenate folder name and file name to play file
    Serial.print("playing track: ");
    Serial.println(track);
    //    playFile(strcat(strcat(dirNameList[track], "/"), fileNameList[track]));  // filenames are always uppercase 8.3 format
    playFile(fList[track]);
    track++;
    track %= numTracks;
    delay(500);

  }

}


void printDirectory(File dir, int numTabs) {
  char* entryName;
  char* lastDirName;
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      Serial.print("found this many valid .wav files: ");
      Serial.println(numTracks);
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
            dirNameList[numTracks][i] = lastDirName[i];
          }
          for (int i = 0; i < 16; i++) {
            fileNameList[numTracks][i] = entryName[i];

          }
          char* dirName = strcat(strcat(dirNameList[numTracks], "/"), fileNameList[numTracks]);
          for (int i = 0; i < 32; i++) {
//            fileNameList[numTracks][i] = entryName[i];
            fList[numTracks][i] = dirName[i];
          }
          
          Serial.print("stored file name: ");
          Serial.print(fList[numTracks]);
          Serial.print(" at loc: ");
          Serial.print(numTracks);
          
          numTracks++;
        }
      }
      Serial.print(entry.name());

      if (entry.isDirectory()) {
        lastDirName = entry.name();
        Serial.println("/");
        printDirectory(entry, numTabs + 1);
      } else {
        // files have sizes, directories do not
        Serial.print("\t\t");
        Serial.println(entry.size(), DEC);
      }
    }

    entry.close();
  }
}
