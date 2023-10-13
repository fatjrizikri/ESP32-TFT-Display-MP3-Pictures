#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <FS.h>
#include <SD.h>
#include <JPEGDecoder.h>
#include "Arduino.h"
#include "Audio.h"
#include "fonts.h"
#include "RTClib.h"
#include "icons.h"     // File header yang berisi definisi gambar

#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18

// I2S Connections
#define I2S_DOUT      12
#define I2S_BCLK      26
#define I2S_LRC       25

#define value_bat 35
#define value_chrg 34

Audio audio;
//wifi
const char* ssid = "Hallo";      // Nama jaringan WiFi
const char* password = "000zikri";  // Kata sandi WiFi
int maxConnectionAttempts = 2;        // Jumlah maksimal percobaan koneksi
int connectionTimeout = 5000;         // Waktu maksimal (dalam milidetik) untuk setiap percobaan
int status_con = 0;
//Rtc
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String tanggal, time_, subuh, dzuhur, ashar, maghrib, isya, date_now, time_now;
int batteryLevel, ind_chrg;
// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();
unsigned long drawTime = 0;
// Pin use esp 32
const int btn_blue = 17;
const int btn_red = 16;
const int btn_yellow = 27;
const int btn_green = 32;

int hh_subuh = 4, hh_dzuhur = 11, hh_ashar = 3, hh_maghrib = 17, hh_isya = 19;
int mm_subuh = 30, mm_dzuhur = 51, mm_ashar = 2, mm_maghrib = 53, mm_isya = 1;
int year_now = 2023, month_now = 10, day_now = 13;
int hh_now = 10, mm_now = 13;
int nh, nm, ns;
int selectedOption = 1;
int cursor_ = 0, cursorSet_ = 0;
char ind_time1, ind_time2, ind_time3, ind_time4, ind_time5;
//icon enter
int coordinate_x = 110;
int coordinate_y = 160;
int coordinate_x_kiri = 200;
int coordinate_y_kiri = 160;
int coordinate_x_kanan = 280;
int coordinate_y_kanan = 160;
int startX, startY, startX_kiri, startX_kanan, startY_kiri, startY_kanan;

// setting icon battery
int batteryWidth = 25; // Lebar indikator baterai
int batteryHeight = 12; // Tinggi indikator baterai
int batteryX = 10; //(tft.width() - batteryWidth) / 2; // Posisi X indikator baterai
int batteryY = 15; // Posisi Y indikator baterai
int batteryBorder = 2; // Ketebalan border indikator baterai
// setting size icons
const int iconsWidth = 24;   // Width of the heart image
const int iconsHeight = 24;  // Height of the heart image
const int logosWidth = 48;   // Width of the heart image
const int logosHeight = 48;  // Height of the heart image
uint32_t color24bit_love, color24bit_electric, color24bit_wifi, color24bit_disconnect;
uint32_t color24bit_home, color24bit_pray, color24bit_musics, color24bit_pictures;
uint8_t r, g, b;
uint16_t color16bit_love, color16bit_electric, color16bit_wifi, color16bit_disconnect;
uint16_t color16bit_home, color16bit_pray, color16bit_musics, color16bit_pictures;

uint16_t icon_love[iconsWidth * iconsHeight];
uint16_t logo_pray[logosWidth * logosHeight];
uint16_t logo_home[logosWidth * logosHeight];
uint16_t logo_musics[logosWidth * logosHeight];
uint16_t logo_pictures[logosWidth * logosHeight];
uint16_t icon_disconnect[iconsWidth * iconsHeight];
uint16_t icon_wifi[iconsWidth * iconsHeight];
// list name file image
const char* imageFiles[] = {"/Pictures/image1.jpg", "/Pictures/image2.jpg", "/Pictures/image3.jpg", "/Pictures/image4.jpg", "/Pictures/image5.jpg"};
const int numImages = 5;
int currentImageIndex = 0;  // Indeks dimulai dari 0
// list name file song
const char* songFiles[] = {"/Musics/Special_Voice.mp3", "/Musics/HappyBirthday.mp3", "/Musics/song1.mp3", "/Musics/song2.mp3", "/Musics/song3.mp3", "/Musics/song4.mp3", "/Musics/song5.mp3"};
const int numSongs = 7;
int currentSongIndex = 0;  // Indeks dimulai dari 0
bool autoPlayMusics = false;
bool BacktoHome = false;

bool prevButtonState = HIGH;
bool nextButtonState = HIGH;
bool TrueNextSong = false;
bool TruePervSong = false;
/////////// With 1024 stars the update rate is ~65 frames per second
#define NSTARS 1024
uint8_t sx[NSTARS] = {};
uint8_t sy[NSTARS] = {};
uint8_t sz[NSTARS] = {};
uint8_t za, zb, zc, zx;
// Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
uint8_t __attribute__((always_inline)) rng()
{
  zx++;
  za = (za ^ zc ^ zx);
  zb = (zb + za);
  zc = ((zc + (zb >> 1))^za);
  return zc;
}
void setup() {
  Serial.begin(115200);
  tft.init();
  rtc.begin();
  tft.setRotation(3);
  unsigned long startTime = millis();
  const unsigned long timeoutDuration = 20000; // Waktu timeout dalam milidetik (20 detik)
  //  rtc.adjust(DateTime(2023, 9, 15, 0, 36, 0));
  pinMode(btn_blue, INPUT_PULLUP);
  pinMode(btn_red, INPUT_PULLUP);
  pinMode(btn_yellow, INPUT_PULLUP);
  pinMode(btn_green, INPUT_PULLUP);
  pinMode(value_bat, INPUT);
  pinMode(value_chrg, INPUT);
  pinMode(22, OUTPUT);
  pinMode(15, OUTPUT);
  digitalWrite(22, HIGH); // Touch controller chip select (if used)
  digitalWrite(15, HIGH); // TFT screen chip select
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  // Initialize SPI bus for microSD Card
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  tft.fillScreen(TFT_BLACK);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // Set Volume
  audio.setVolume(0);
  for (int attempts = 0; attempts < maxConnectionAttempts; attempts++) {
    Serial.printf("Attempt %d to connect to WiFi...\n", attempts + 1);
    connectToWiFi();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(F("Connected to WiFi"));
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setFreeFont(FF41);
      tft.drawString("Connected to WiFi", 160, 120, GFXFF);
      break;  // Keluar dari loop jika berhasil terhubung
    }
    if (attempts < maxConnectionAttempts - 1) {
      Serial.printf("Failed to connect. Retrying in %d seconds...\n", connectionTimeout / 1000);
      delay(1000);
    } else {
      Serial.println(F("Failed to connect after maximum attempts."));
      delay(100);
    }
  }
  if (!SD.begin()) {
    Serial.println(F("Card Mount Failed"));
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println(F("No SD card attached"));
    return;
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);
  tft.fillScreen(TFT_BLACK);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (rtc.lostPower()) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    // Set text colour to orange with black background
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setFreeFont(FF41);
    tft.drawString("RTC lost power !!!", 160, 90, GFXFF);
    tft.setFreeFont(FF41);
    tft.drawString("Set Date & Time", 160, 120, GFXFF);
    delay(1000);
    tft.fillScreen(TFT_BLACK);
    setting();
    rtc.adjust(DateTime(year_now, month_now, day_now, hh_now, mm_now, 0));
    //    rtc.adjust(DateTime(2023, 9, 15, 0, 27, 0));
  }
  while (millis() - startTime < timeoutDuration) {
    Starfield();
  }
  // Set text datum to middle centre
  tft.setTextDatum(MC_DATUM);
  // Set text colour to orange with black background
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF41);
  tft.drawString("I LOVE YOU", 160, 120, GFXFF);
  delay(1000);
  // Setup I2S
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // Set Volume
  audio.setVolume(15);
  // Open music file
  audio.connecttoFS(SD, "/Setup_Musics/music_start.mp3");
  tft.fillScreen(TFT_BLACK);
}

void loop()
{
  while (audio.isRunning()) {
    audio.loop();  // Play the next song if the current one has finished
  }
  utama();
  buzzer_salat();
  if (digitalRead(btn_red) == LOW) {
    tft.fillScreen(TFT_BLACK);
    time_salat();
    delay(100); // Debouncing
  }
  if (digitalRead(btn_yellow) == LOW) {
    tft.fillScreen(TFT_BLACK);
    musics();
    delay(100); // Debouncing
  }
  if (digitalRead(btn_green) == LOW) {
    tft.fillScreen(TFT_BLACK);
    information_images();
    delay(100); // Debouncing
  }
}
void utama() {
  head_icons();
  buttom_menu_logos();
  unsigned long refresh = millis();
  const unsigned long timeoutDuration_refresh = 1000; // Waktu timeout dalam milidetik (20 detik)
  DateTime now = rtc.now();
  tanggal =  String (daysOfTheWeek[now.dayOfTheWeek()]) + " " + String(now.day()) + " / " + String(now.month()) + " / " + String(now.year());
  time_ = String(now.hour()) + " : " + String(now.minute()) + " : " + String(now.second());
  // Convert 24-bit color to 16-bit color (5 bits for red, 6 bits for green, 5 bits for blue)
  if (millis() - refresh < timeoutDuration_refresh) {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FF41);
    tft.drawString(tanggal, 160, 80, GFXFF);
    tft.setFreeFont(FF41);
    tft.drawString(time_, 160, 120, GFXFF);
    delay(1000);
    tft.fillRect(40, 70, 240, 70, TFT_BLACK); //tft.fillRect(x, y, width, height, TFT_BLACK); Isi persegi panjang dengan warna merah
  }
}
void information_images() {
  head_icons();
  buttom_icons_images();
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FF41);
  tft.drawString("INFORMATION ...", 160, 70, GFXFF);
  tft.setFreeFont(FF41);
  tft.drawString("In This Device Have 5 Pictures", 160, 110, GFXFF);
  tft.setFreeFont(FF41);
  tft.drawString("Feature Button Images ", 160, 130, GFXFF);
  delay(5000);
  tft.fillScreen(TFT_BLACK);
  images();
}
void musics() {
  while (true) {
    head_icons();
    icons_musics();
    while (audio.isRunning() && autoPlayMusics) {
      audio.loop();
      //      Serial.println("status: " + String(autoPlayMusics));
      if (digitalRead(btn_red) == LOW) {
        autoPlayMusics = false;
        audio.stopSong();
        break;
        delay(300); // Debouncing
      }
      if (digitalRead(btn_blue) == LOW) {
        autoPlayMusics = false;
        BacktoHome = true;
        tft.fillScreen(TFT_BLACK);
        audio.stopSong();
        break;
        delay(300); // Debouncing
      }
    }
    while (audio.isRunning() && !autoPlayMusics) {
      audio.loop();  // Play the next song if the current one has finished
      if (digitalRead(btn_yellow) == LOW) {
        audio.stopSong();
        TruePervSong = true;
        break;
        delay(300);  // Add a delay to avoid multiple rapid presses
      }
      // Check if the NEXT button is pressed
      if (digitalRead(btn_green) == LOW) {
        audio.stopSong();
        TrueNextSong = true;
        break;
        delay(300);  // Add a delay to avoid multiple rapid presses
      }
      if (digitalRead(btn_red) == LOW) {
        tft.fillRect(10, 100, 350, 80, TFT_BLACK);
        audio.stopSong();
        autoPlayMusics = false;
        break;
        delay(300); // Debouncing
      }
      if (digitalRead(btn_blue) == LOW) {
        autoPlayMusics = false;
        BacktoHome = true;
        tft.fillScreen(TFT_BLACK);
        audio.stopSong();
        break;
        delay(300); // Debouncing
      }
    }
    if (autoPlayMusics && !audio.isRunning()) {
      tft.fillRect(10, 100, 350, 80, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Automatic Play Musics", 160, 150, GFXFF);
      delay(300);
      currentSongIndex = (currentSongIndex + 1) % numSongs;
      audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
      audio.setVolume(15);
      audio.connecttoFS(SD, songFiles[currentSongIndex]);
      Serial.println(songFiles[currentSongIndex]);
      //  tft.fillRect(10, 100, 350, 30, TFT_BLACK); //tft.fillRect(x, y, width, height, TFT_BLACK); Isi persegi panjang dengan warna merah
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setFreeFont(FF41);
      tft.drawString("INFORMATION ...", 160, 110, GFXFF);
      tft.setFreeFont(FF41);
      tft.drawString(songFiles[currentSongIndex], 160, 130, GFXFF);
    }
    if (!autoPlayMusics) {
      if (!audio.isRunning()) {
        audio.stopSong();
        tft.fillRect(10, 100, 350, 80, TFT_BLACK); //tft.fillRect(x, y, width, height, TFT_BLACK); Isi persegi panjang dengan warna merah
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(FF41);
        tft.drawString("INFORMATION ...", 160, 120, GFXFF);
        tft.setFreeFont(FF41);
        tft.drawString("Press Button to Play Music", 160, 150, GFXFF);
        delay(300);
        if (digitalRead(btn_red) == LOW) {
          audio.stopSong();
          autoPlayMusics = true;
          delay(300); // Debouncing
        }
        if (digitalRead(btn_yellow) == LOW) {
          audio.stopSong();
          tft.fillRect(10, 100, 350, 80, TFT_BLACK);
          currentSongIndex = (currentSongIndex - 1 + numSongs) % numSongs;
          playPrevSong();
          delay(300);  // Add a delay to avoid multiple rapid presses
        }
        // Check if the NEXT button is pressed
        if (digitalRead(btn_green) == LOW) {
          audio.stopSong();
          tft.fillRect(10, 100, 350, 80, TFT_BLACK);
          currentSongIndex = (currentSongIndex + 1) % numSongs;
          playNextSong();
          delay(300);  // Add a delay to avoid multiple rapid presses
        }
      }
      if (TruePervSong) {
        TruePervSong = false;
        tft.fillRect(10, 100, 350, 80, TFT_BLACK);
        currentSongIndex = (currentSongIndex - 1 + numSongs) % numSongs;
        playPrevSong();
        delay(300);  // Add a delay to avoid multiple rapid presses
      }
      if (TrueNextSong) {
        TrueNextSong = false;
        tft.fillRect(10, 100, 350, 80, TFT_BLACK);
        currentSongIndex = (currentSongIndex + 1) % numSongs;
        playNextSong();
        delay(300);  // Add a delay to avoid multiple rapid presses
      }
      if (digitalRead(btn_red) == LOW) {
        audio.stopSong();
        delay(300); // Debouncing
      }
    }
    if (digitalRead(btn_red) == LOW && autoPlayMusics) {
      audio.stopSong();
      autoPlayMusics = false;
      delay(300); // Debouncing
    }
    if (digitalRead(btn_blue) == LOW) {
      autoPlayMusics = false;
      BacktoHome = true;
      tft.fillScreen(TFT_BLACK);
      audio.stopSong();
      delay(300); // Debouncing
    }
    if (BacktoHome) {
      autoPlayMusics = false;
      BacktoHome = false;
      tft.fillScreen(TFT_BLACK);
      break;
    }
  }
}
void playNextSong() {
  // Increment the current song index and loop back to the first song if needed
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // Set Volume
  audio.setVolume(15);
  // Open the previous music file
  audio.connecttoFS(SD, songFiles[currentSongIndex]);
  Serial.println(songFiles[currentSongIndex]);
  //  tft.fillRect(10, 100, 350, 30, TFT_BLACK); //tft.fillRect(x, y, width, height, TFT_BLACK); Isi persegi panjang dengan warna merah
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FF41);
  tft.drawString("INFORMATION ...", 160, 110, GFXFF);
  tft.setFreeFont(FF41);
  tft.drawString(songFiles[currentSongIndex], 160, 130, GFXFF);
}

void playPrevSong() {
  // Decrement the current song index and loop to the last song if needed
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // Set Volume
  audio.setVolume(15);
  // Open the previous music file
  audio.connecttoFS(SD, songFiles[currentSongIndex]);
  //  tft.fillRect(10, 100, 350, 30, TFT_BLACK); //tft.fillRect(x, y, width, height, TFT_BLACK); Isi persegi panjang dengan warna merah
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FF41);
  tft.drawString("INFORMATION ...", 160, 110, GFXFF);
  tft.setFreeFont(FF41);
  tft.drawString(songFiles[currentSongIndex], 160, 130, GFXFF);
}

void images() {
  while (true) {
    bool nextButtonState = digitalRead(btn_green);
    bool prevButtonState = digitalRead(btn_yellow);

    if (nextButtonState == LOW && prevButtonState == HIGH) {
      currentImageIndex = (currentImageIndex + 1) % numImages;
    }

    if (prevButtonState == LOW && nextButtonState == HIGH) {
      currentImageIndex = (currentImageIndex - 1 + numImages) % numImages;
    }
    if (digitalRead(btn_red) == LOW) {
      tft.fillScreen(TFT_BLACK);
      time_salat();
      break;
      delay(100); // Debouncing
    }
    if (digitalRead(btn_blue) == LOW) {
      tft.fillScreen(TFT_BLACK);
      loop();
      break;
      delay(100); // Debouncing
    }
    prevButtonState = nextButtonState;
    displayImage(imageFiles[currentImageIndex]);
  }
}
void displayImage(const char* filename) {
  tft.setRotation(3);
  tft.fillScreen(random(0xFFFF));
  int x = (tft.width() - 320) / 2 - 1;
  int y = (tft.height() - 240) / 2 - 1;
  drawSdJpeg(filename, x, y);
  delay(2000);
}
void drawSdJpeg(const char *filename, int xpos, int ypos) {
  // Open the named file (the Jpeg decoder library will close it)
  File jpegFile = SD.open( filename, FILE_READ);  // or, file handle reference for SD library
  if ( !jpegFile ) {
    Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
    return;
  }
  Serial.println("===========================");
  Serial.print("Drawing file: "); Serial.println(filename);
  Serial.println("===========================");

  // Use one of the following methods to initialise the decoder:
  bool decoded = JpegDec.decodeSdFile(jpegFile);  // Pass the SD file handle to the decoder,
  //bool decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)

  if (decoded) {
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  }
  else {
    Serial.println("Jpeg file format not supported!");
  }
}
void jpegRender(int xpos, int ypos) {
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;
  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);
  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
  uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);
  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;
  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();
  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;
  // Fetch data from the file, decode and display
  while (JpegDec.read()) {    // While there is more data in the file
    pImg = JpegDec.pImage ;   // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)
    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;
    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;
    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;
    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }
    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;
    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ( (mcu_y + win_h) >= tft.height())
      JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }
  tft.setSwapBytes(swapBytes);
  showTime(millis() - drawTime); // These lines are for sketch testing only
}
void showTime(uint32_t msTime) {
  Serial.print(F(" JPEG drawn in "));
  Serial.print(msTime);
  Serial.println(F(" ms "));
}
void time_salat() {
  while (true) {
    unsigned long refresh = millis();
    const unsigned long timeoutDuration_refresh = 1000; // Waktu timeout dalam milidetik (20 detik)
    head_icons();
    ind_waktu_salat();
    buttom_fetures_logos();
    subuh = String(ind_time1) + " Subuh  " + String(hh_subuh) + " : " + String(mm_subuh) + " : 0";
    dzuhur = String(ind_time2) + " dzuhur  " + String(hh_dzuhur) + " : " + String(mm_dzuhur) + " : 0";
    ashar = String(ind_time3) + " ashar  " + String(hh_ashar) + " : " + String(mm_ashar) + " : 0";
    maghrib = String(ind_time4) + " maghrib  " + String(hh_maghrib) + " : " + String(mm_maghrib) + " : 0";
    isya = String(ind_time5) + " isya  " + String(hh_isya) + " : " + String(mm_isya) + " : 0";
    if (millis() - refresh < timeoutDuration_refresh) {
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setFreeFont(FF41);
      tft.drawString(subuh, 160, 50, GFXFF);
      tft.setFreeFont(FF41);
      tft.drawString(dzuhur, 160, 80, GFXFF);
      tft.setFreeFont(FF41);
      tft.drawString(ashar, 160, 110, GFXFF);
      tft.setFreeFont(FF41);
      tft.drawString(maghrib, 160, 140, GFXFF);
      tft.setFreeFont(FF41);
      tft.drawString(isya, 160, 170, GFXFF);
      delay(1000);
      tft.fillRect(60, 40, 200, 145, TFT_BLACK); //tft.fillRect(x, y, width, height, TFT_BLACK); Isi persegi panjang dengan warna merah
    }
    if (digitalRead(btn_blue) == LOW) {
      tft.fillScreen(TFT_BLACK);
      break;
      delay(100); // Debouncing
    }
    if (digitalRead(btn_green) == LOW) {
      if (selectedOption >= 5) {
        selectedOption = 1;
      } else {
        selectedOption = selectedOption + 1;
      }
      delay(100);
    }
    if (digitalRead(btn_yellow) == LOW) {
      if (selectedOption <= 1) {
        selectedOption = 5;
      } else {
        selectedOption = selectedOption - 1;
      }
      delay(100);
    }  if (digitalRead(btn_red) == LOW) {
      tft.fillScreen(TFT_BLACK);
      setAlarmTime(selectedOption);
      delay(100); // Debouncing
    }
  }
}
void ind_waktu_salat() {
  if (selectedOption == 1) {
    ind_time1 = '-'; ind_time2 = ' '; ind_time3 = ' '; ind_time4 = ' '; ind_time5 = ' ';
  }
  if (selectedOption == 2) {
    ind_time1 = ' '; ind_time2 = '-'; ind_time3 = ' '; ind_time4 = ' '; ind_time5 = ' ';
  }
  if (selectedOption == 3) {
    ind_time1 = ' '; ind_time2 = ' '; ind_time3 = '-'; ind_time4 = ' '; ind_time5 = ' ';
  }
  if (selectedOption == 4) {
    ind_time1 = ' '; ind_time2 = ' '; ind_time3 = ' '; ind_time4 = '-'; ind_time5 = ' ';
  }
  if (selectedOption == 5) {
    ind_time1 = ' '; ind_time2 = ' '; ind_time3 = ' '; ind_time4 = ' '; ind_time5 = '-';
  }
  Serial.print(F("Selected option: "));
  Serial.println(selectedOption);
}
void icons_musics() {
  startX = 60 + coordinate_x; // X coordinate for the starting point of the icon == x
  startY = 100 + coordinate_y; // Y coordinate for the starting point of the icon == y

  startX_kiri = 60 + coordinate_x_kiri; // X coordinate for the starting point of the icon == x
  startY_kiri = 100 + coordinate_y_kiri; // Y coordinate for the starting point of the icon == y

  startX_kanan = 60 + coordinate_x_kanan; // X coordinate for the starting point of the icon == x
  startY_kanan = 100 + coordinate_y_kanan; // Y coordinate for the starting point of the icon == y
  for (int i = 0; i < logosWidth * logosHeight; i++) {
    color24bit_home = image_data_home[i];
    r = color24bit_home & 0xFF; // Red component //green
    g = color24bit_home & 0xFF;  // Green component //blue
    b = color24bit_home  & 0xFF;  // Blue component //red
    color16bit_home = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    logo_home[i] = color16bit_home;
  }
  for (int i = 0; i < logosWidth * logosHeight; i++) {
    color24bit_musics = image_data_music[i];
    r = color24bit_musics & 0xFF; // Red component //green
    g = color24bit_musics & 0xFF;  // Green component //blue
    b = color24bit_musics  & 0xFF;  // Blue component //red
    color16bit_musics = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    logo_musics[i] = color16bit_musics;
  }
  //kiri
  tft.fillTriangle(startX_kiri - 60, startY_kiri - 50, startX_kiri - 50, startY_kiri - 70, startX_kiri - 50, startY_kiri - 30, TFT_WHITE); // (60, 100, 80, 80, 40, 120)
  //kanan
  tft.fillTriangle(startX_kanan - 40, startY_kanan - 50, startX_kanan - 50, startY_kanan - 70, startX_kanan - 50, startY_kanan - 30, TFT_WHITE);
  tft.pushImage(10, 180, logosWidth, logosHeight, logo_home);
  tft.pushImage(90, 180, logosWidth, logosHeight, logo_musics);
}
void buttom_fetures_logos() {
  startX = 60 + coordinate_x; // X coordinate for the starting point of the icon == x
  startY = 100 + coordinate_y; // Y coordinate for the starting point of the icon == y

  startX_kiri = 60 + coordinate_x_kiri; // X coordinate for the starting point of the icon == x
  startY_kiri = 100 + coordinate_y_kiri; // Y coordinate for the starting point of the icon == y

  startX_kanan = 60 + coordinate_x_kanan; // X coordinate for the starting point of the icon == x
  startY_kanan = 100 + coordinate_y_kanan; // Y coordinate for the starting point of the icon == y
  //kiri
  tft.fillTriangle(startX_kiri - 60, startY_kiri - 50, startX_kiri - 50, startY_kiri - 70, startX_kiri - 50, startY_kiri - 30, TFT_WHITE); // (60, 100, 80, 80, 40, 120)
  //kanan
  tft.fillTriangle(startX_kanan - 40, startY_kanan - 50, startX_kanan - 50, startY_kanan - 70, startX_kanan - 50, startY_kanan - 30, TFT_WHITE);
  // Draw the upper part of the icon enter
  tft.fillTriangle(startX - 60, startY - 50, startX - 50, startY - 70, startX - 50, startY - 30, TFT_WHITE); // (60, 100, 80, 80, 40, 120)
  tft.fillRect(startX - 28, startY - 60, 3 , 10, TFT_WHITE);
  tft.fillRect(startX - 50, startY - 50, 25, 3, TFT_WHITE); //hrozontal
  for (int i = 0; i < logosWidth * logosHeight; i++) {
    color24bit_home = image_data_home[i];
    r = color24bit_home & 0xFF; // Red component //green
    g = color24bit_home & 0xFF;  // Green component //blue
    b = color24bit_home  & 0xFF;  // Blue component //red
    color16bit_home = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    logo_home[i] = color16bit_home;
  }
  tft.pushImage(10, 180, logosWidth, logosHeight, logo_home);
}
void buttom_menu_logos() {
  for (int i = 0; i < logosWidth * logosHeight; i++) {
    color24bit_home = image_data_home[i];
    r = color24bit_home & 0xFF; // Red component //green
    g = color24bit_home & 0xFF;  // Green component //blue
    b = color24bit_home  & 0xFF;  // Blue component //red
    color16bit_home = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    logo_home[i] = color16bit_home;
  }
  for (int i = 0; i < logosWidth * logosHeight; i++) {
    color24bit_pray = image_data_salat[i];
    r = color24bit_pray & 0xFF; // Red component //green
    g = color24bit_pray & 0xFF;  // Green component //blue
    b = color24bit_pray  & 0xFF;  // Blue component //red
    color16bit_pray = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    logo_pray[i] = color16bit_pray;
  }
  for (int i = 0; i < logosWidth * logosHeight; i++) {
    color24bit_musics = image_data_music[i];
    r = color24bit_musics & 0xFF; // Red component //green
    g = color24bit_musics & 0xFF;  // Green component //blue
    b = color24bit_musics  & 0xFF;  // Blue component //red
    color16bit_musics = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    logo_musics[i] = color16bit_musics;
  }
  for (int i = 0; i < logosWidth * logosHeight; i++) {
    color24bit_pictures = image_data_pictures[i];
    r = color24bit_pictures & 0xFF; // Red component //green
    g = color24bit_pictures & 0xFF;  // Green component //blue
    b = color24bit_pictures  & 0xFF;  // Blue component //red
    color16bit_pictures = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    logo_pictures[i] = color16bit_pictures;
  }
  // Display the image
  tft.pushImage(10, 180, logosWidth, logosHeight, logo_home);
  tft.pushImage(90, 185, logosWidth, logosHeight, logo_pray);
  tft.pushImage(170, 180, logosWidth, logosHeight, logo_musics);
  tft.pushImage(250, 180, logosWidth, logosHeight, logo_pictures);
}
void buttom_icons_images() {
  startX = 60 + coordinate_x; // X coordinate for the starting point of the icon == x
  startY = 100 + coordinate_y; // Y coordinate for the starting point of the icon == y

  startX_kiri = 60 + coordinate_x_kiri; // X coordinate for the starting point of the icon == x
  startY_kiri = 100 + coordinate_y_kiri; // Y coordinate for the starting point of the icon == y

  startX_kanan = 60 + coordinate_x_kanan; // X coordinate for the starting point of the icon == x
  startY_kanan = 100 + coordinate_y_kanan; // Y coordinate for the starting point of the icon == y
  for (int i = 0; i < logosWidth * logosHeight; i++) {
    color24bit_home = image_data_home[i];
    r = color24bit_home & 0xFF; // Red component //green
    g = color24bit_home & 0xFF;  // Green component //blue
    b = color24bit_home  & 0xFF;  // Blue component //red
    color16bit_home = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    logo_home[i] = color16bit_home;
  }
  for (int i = 0; i < logosWidth * logosHeight; i++) {
    color24bit_pray = image_data_salat[i];
    r = color24bit_pray & 0xFF; // Red component //green
    g = color24bit_pray & 0xFF;  // Green component //blue
    b = color24bit_pray  & 0xFF;  // Blue component //red
    color16bit_pray = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    logo_pray[i] = color16bit_pray;
  }
  //kiri
  tft.fillTriangle(startX_kiri - 60, startY_kiri - 50, startX_kiri - 50, startY_kiri - 70, startX_kiri - 50, startY_kiri - 30, TFT_WHITE); // (60, 100, 80, 80, 40, 120)
  //kanan
  tft.fillTriangle(startX_kanan - 40, startY_kanan - 50, startX_kanan - 50, startY_kanan - 70, startX_kanan - 50, startY_kanan - 30, TFT_WHITE);
  tft.pushImage(10, 180, logosWidth, logosHeight, logo_home);
  tft.pushImage(90, 185, logosWidth, logosHeight, logo_pray);
}
void head_icons() {
  ind_chrg = digitalRead(value_chrg);
  batteryLevel = map(analogRead(value_bat), 0, 4095, 0, 100);
  Serial.print("ind_chrg : " + String(ind_chrg));
  Serial.println(" batteryLavel : " + String(batteryLevel));
  tft.fillRect(batteryX, batteryY, batteryWidth, batteryHeight, TFT_WHITE);
  tft.fillRect(batteryX + batteryWidth, batteryY + batteryBorder, 5, batteryHeight - (2 * batteryBorder), TFT_WHITE);

  // Hitung panjang indikator baterai yang diisi berdasarkan tingkat baterai
  int filledWidth = map(batteryLevel, 0, 100, 0, batteryWidth - (2 * batteryBorder));
  tft.fillRect(batteryX + batteryBorder, batteryY + batteryBorder, filledWidth, batteryHeight - (2 * batteryBorder), TFT_GREEN);

  if (ind_chrg != 1) {
    tft.fillRect(50, 10, 30, 25, TFT_BLACK);
  }
  if (ind_chrg == 1 ) {
    uint16_t icon_electric[iconsWidth * iconsHeight];
    for (int i = 0; i < iconsWidth * iconsHeight; i++) {
      color24bit_electric = image_data_electric[i];
      r = color24bit_electric & 0xFF; // Red component //green
      g = color24bit_electric & 0x00;  // Green component //blue
      b = (color24bit_electric >> 8) & 0xFF;  // Blue component //red
      color16bit_electric = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
      icon_electric[i] = color16bit_electric;
    }
    // Display the image
    tft.pushImage(50, 10, iconsWidth, iconsHeight, icon_electric);
  }
  if (WiFi.status() == WL_CONNECTED) {
    for (int i = 0; i < iconsWidth * iconsHeight; i++) {
      color24bit_wifi = image_data_wifi[i];
      uint8_t r = color24bit_wifi & 0xFF; // Red component
      uint8_t g = color24bit_wifi & 0xFF;  // Green component
      uint8_t b = color24bit_wifi & 0xFF;         // Blue component
      color16bit_wifi = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
      icon_wifi[i] = color16bit_wifi;
    }

    // Display the image
    tft.pushImage(280, 10, iconsWidth, iconsHeight, icon_wifi);
  }
  if (WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i < iconsWidth * iconsHeight; i++) {
      color24bit_disconnect = image_data_icons8offline48[i];
      r = color24bit_disconnect & 0xFF; // Red component //green
      g = color24bit_disconnect & 0xFF;  // Green component //blue
      b = color24bit_disconnect & 0xFF;  // Blue component //red
      color16bit_disconnect = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
      icon_disconnect[i] = color16bit_disconnect;
    }
    // Display the image
    tft.pushImage(280, 10, iconsWidth, iconsHeight, icon_disconnect);
  }
  // Icon LOVE
  for (int i = 0; i < iconsWidth * iconsHeight; i++) {
    color24bit_love = image_data_Hearticons02[i];
    r = color24bit_love & 0xFF; // Red component //green
    g = color24bit_love & 0xFF;  // Green component //blue
    b = (color24bit_love >> 16) & 0xFF;  // Blue component //red
    color16bit_love = ((g & 0xF8) << 8) | ((b & 0xFC) << 3) | (r >> 3);
    icon_love[i] = color16bit_love;
  }
  tft.pushImage(240, 10, iconsWidth, iconsHeight, icon_love);
}
void connectToWiFi() {
  WiFi.begin(ssid, password);
  int elapsedTime = 0;
  while (WiFi.status() != WL_CONNECTED && elapsedTime < connectionTimeout) {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FF41);
    tft.drawString("Connecting...", 160, 120, GFXFF);
    delay(500);
    elapsedTime += 500;
  }
}
void Starfield() {

  unsigned long t0 = micros();
  uint8_t spawnDepthVariation = 255;

  for (int i = 0; i < NSTARS; ++i)
  {
    if (sz[i] <= 1)
    {
      sx[i] = 160 - 120 + rng();
      sy[i] = rng();
      sz[i] = spawnDepthVariation--;
    }
    else
    {
      int old_screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
      int old_screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;
      tft.drawPixel(old_screen_x, old_screen_y, TFT_BLACK);

      sz[i] -= 2;
      if (sz[i] > 1)
      {
        int screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
        int screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;

        if (screen_x >= 0 && screen_y >= 0 && screen_x < 320 && screen_y < 240)
        {
          uint8_t r, g, b;
          r = g = b = 255 - sz[i];
          tft.drawPixel(screen_x, screen_y, tft.color565(r, g, b));
        }
        else
          sz[i] = 0; // Out of screen, die.
      }
    }
  }
  unsigned long t1 = micros();
  Serial.println(1.0 / ((t1 - t0) / 1000000.0));
}

void setAlarmTime(int option) {
  isya = String(hh_isya) + " : " + String(mm_isya) + " : 0";
  Serial.print("Setting alarm time for option ");
  Serial.println(option);
  switch (option) {
    case 1: //Subuh
      while (true) {
        head_icons();
        subuh = String(hh_subuh) + " : " + String(mm_subuh) + " : 0";
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(FF41);
        tft.drawString("=== Subuh ===", 160, 60, GFXFF);
        tft.drawString(subuh, 160, 100, GFXFF);
        if (digitalRead(btn_red) == LOW) {
          cursor_ = cursor_ + 1;
          tft.fillScreen(TFT_BLACK);
          delay(100);
        }
        if (digitalRead(btn_blue) == LOW) {
          tft.fillScreen(TFT_BLACK);
          cursor_ = 0;
          delay(300);
          break;
        }
        if (cursor_ == 1) { //hour
          tft.drawString("Hour", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (hh_subuh >= 23) {
              hh_subuh = 0;
            } else {
              hh_subuh = hh_subuh + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (hh_subuh <= 0) {
              hh_subuh = 23;
            } else {
              hh_subuh = hh_subuh - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 2) { //menute
          tft.drawString("Minute", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (mm_subuh >= 59) {
              mm_subuh = 0;
            } else {
              mm_subuh = mm_subuh + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (mm_subuh <= 0) {
              mm_subuh = 59;
            } else {
              mm_subuh = mm_subuh - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 3) {
          cursor_ = 0;
          tft.fillScreen(TFT_BLACK);
          delay(300);
          break;
        }
        Serial.println("time subuh");
        Serial.println("subuh : " + String(hh_subuh) + " : " + String(mm_subuh));
        Serial.print("Cursor: ");
        Serial.println(cursor_);
        delay(1000);
      }
      break;
    case 2: //Dzuhur
      while (true) {
        head_icons();
        dzuhur = String(hh_dzuhur) + " : " + String(mm_dzuhur) + " : 0";
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(FF41);
        tft.drawString("=== Dzuhur ===", 160, 60, GFXFF);
        tft.drawString(dzuhur, 160, 100, GFXFF);
        if (digitalRead(btn_red) == LOW) {
          cursor_ = cursor_ + 1;
          tft.fillScreen(TFT_BLACK);
          delay(100);
        }
        if (digitalRead(btn_blue) == LOW) {
          tft.fillScreen(TFT_BLACK);
          cursor_ = 0;
          delay(300);
          break;
        }
        if (cursor_ == 1) { //hour
          tft.drawString("Hour", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (hh_dzuhur >= 23) {
              hh_dzuhur = 0;
            } else {
              hh_dzuhur = hh_dzuhur + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (hh_dzuhur <= 0) {
              hh_dzuhur = 23;
            } else {
              hh_dzuhur = hh_dzuhur - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 2) { //menute
          tft.drawString("Minute", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (mm_dzuhur >= 59) {
              mm_dzuhur = 0;
            } else {
              mm_dzuhur = mm_dzuhur + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (mm_dzuhur <= 0) {
              mm_dzuhur = 59;
            } else {
              mm_dzuhur = mm_dzuhur - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 3) {
          tft.fillScreen(TFT_BLACK);
          cursor_ = 0;
          delay(300);
          break;
        }
        Serial.println("time Dzuhur");
        Serial.println("dzuhur : " + String(hh_dzuhur) + " : " + String(mm_dzuhur));
        Serial.print("cursor : ");
        Serial.println(cursor_);
        delay(1000);
      }
      break;
    case 3:
      while (true) {
        head_icons();
        ashar = String(hh_ashar) + " : " + String(mm_ashar) + " : 0";
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(FF41);
        tft.drawString("=== Ashar ===", 160, 60, GFXFF);
        tft.drawString(ashar, 160, 100, GFXFF);
        if (digitalRead(btn_red) == LOW) {
          cursor_ = cursor_ + 1;
          tft.fillScreen(TFT_BLACK);
          delay(100);
        }
        if (digitalRead(btn_blue) == LOW) {
          tft.fillScreen(TFT_BLACK);
          cursor_ = 0;
          delay(300);
          break;
        }
        if (cursor_ == 1) { //hour
          tft.drawString("Hour", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (hh_ashar >= 23) {
              hh_ashar = 0;
            } else {
              hh_ashar = hh_ashar + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (hh_ashar <= 0) {
              hh_ashar = 23;
            } else {
              hh_ashar = hh_ashar - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 2) { //menute
          tft.drawString("Minute", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (mm_ashar >= 59) {
              mm_ashar = 0;
            } else {
              mm_ashar = mm_ashar + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (mm_ashar <= 0) {
              mm_ashar = 59;
            } else {
              mm_ashar = mm_ashar - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 3) {
          tft.fillScreen(TFT_BLACK);
          cursor_ = 0;
          delay(300);
          break;
        }
        Serial.println("time Ashar");
        Serial.println("ashar : " + String(hh_ashar) + " : " + String(mm_ashar));
        Serial.print("cursor : ");
        Serial.println(cursor_);
        delay(1000);
      }
      break;
    case 4:
      while (true) {
        head_icons();
        maghrib = String(hh_maghrib) + " : " + String(mm_maghrib) + " : 0";
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(FF41);
        tft.drawString("=== Maghrib ===", 160, 60, GFXFF);
        tft.drawString(maghrib, 160, 100, GFXFF);
        if (digitalRead(btn_red) == LOW) {
          cursor_ = cursor_ + 1;
          tft.fillScreen(TFT_BLACK);
          delay(100);
        }
        if (digitalRead(btn_blue) == LOW) {
          tft.fillScreen(TFT_BLACK);
          cursor_ = 0;
          delay(300);
          break;
        }
        if (cursor_ == 1) { //hour
          tft.drawString("Hour", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (hh_maghrib >= 23) {
              hh_maghrib = 0;
            } else {
              hh_maghrib = hh_maghrib + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (hh_maghrib <= 0) {
              hh_maghrib = 23;
            } else {
              hh_maghrib = hh_maghrib - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 2) { //menute
          tft.drawString("Minute", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (mm_maghrib >= 59) {
              mm_maghrib = 0;
            } else {
              mm_maghrib = mm_maghrib + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (mm_maghrib <= 0) {
              mm_maghrib = 59;
            } else {
              mm_maghrib = mm_maghrib - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 3) {
          tft.fillScreen(TFT_BLACK);
          cursor_ = 0;
          delay(300);
          break;
        }
        Serial.println("time Maghrib");
        Serial.println("maghrib : " + String(hh_maghrib) + " : " + String(mm_maghrib));
        Serial.print("cursor : ");
        Serial.println(cursor_);
        delay(1000);
      }
      break;
    case 5:
      while (true) {
        head_icons();
        isya = String(hh_isya) + " : " + String(mm_isya) + " : 0";
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(FF41);
        tft.drawString("=== Isya ===", 160, 60, GFXFF);
        tft.drawString(isya, 160, 100, GFXFF);
        if (digitalRead(btn_red) == LOW) {
          cursor_ = cursor_ + 1;
          tft.fillScreen(TFT_BLACK);
          delay(100);
        }
        if (digitalRead(btn_blue) == LOW) {
          tft.fillScreen(TFT_BLACK);
          cursor_ = 0;
          delay(300);
          break;
        }
        if (cursor_ == 1) { //hour
          tft.drawString("Hour", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (hh_isya >= 23) {
              hh_isya = 0;
            } else {
              hh_isya = hh_isya + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (hh_isya <= 0) {
              hh_isya = 23;
            } else {
              hh_isya = hh_isya - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 2) { //menute
          tft.drawString("Minute", 160, 140, GFXFF);
          if (digitalRead(btn_green) == LOW) {
            if (mm_isya >= 59) {
              mm_isya = 0;
            } else {
              mm_isya = mm_isya + 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
          if (digitalRead(btn_yellow) == LOW) {
            if (mm_isya <= 0) {
              mm_isya = 59;
            } else {
              mm_isya = mm_isya - 1;
            }
            tft.fillScreen(TFT_BLACK);
            delay(100);
          }
        }
        if (cursor_ == 3) {
          tft.fillScreen(TFT_BLACK);
          cursor_ = 0;
          delay(300);
          break;
        }
        Serial.println("time Isya");
        Serial.println("isya : " + String(hh_isya) + " : " + String(mm_isya));
        Serial.print("cursor : ");
        Serial.println(cursor_);
        delay(1000);
      }
      break;
    default:
      break;
  }
}
void setting() {
  while (true) {
    buttom_fetures_logos();
    date_now = String(year_now) + " - " + String(month_now) + " - " + String(day_now);
    time_now = String(hh_now) + " : " + String(mm_now) + " : 0";
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FF41);
    tft.drawString(date_now, 160, 60, GFXFF);
    tft.drawString(time_now, 160, 100, GFXFF);
    if (digitalRead(btn_red) == LOW) {
      cursorSet_ = cursorSet_ + 1;
      tft.fillScreen(TFT_BLACK);
      delay(100);
    }
    if (digitalRead(btn_blue) == LOW) {
      tft.fillScreen(TFT_BLACK);
      cursorSet_ = 0;
      delay(300);
      break;
    }
    if (cursorSet_ == 1) { //years
      tft.drawString("Year", 160, 140, GFXFF);
      if (digitalRead(btn_green) == LOW) {
        if (year_now >= 2030) {
          year_now = 2022;
        } else {
          year_now = year_now + 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
      if (digitalRead(btn_yellow) == LOW) {
        if (year_now <= 2022) {
          year_now = 2030;
        } else {
          year_now = year_now - 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
    }
    if (cursorSet_ == 2) { //menute
      tft.drawString("Month", 160, 140, GFXFF);
      if (digitalRead(btn_green) == LOW) {
        if (month_now >= 12) {
          month_now = 1;
        } else {
          month_now = month_now + 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
      if (digitalRead(btn_yellow) == LOW) {
        if (month_now <= 1) {
          month_now = 12;
        } else {
          month_now = month_now - 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
    }
    if (cursorSet_ == 3) {
      tft.drawString("Day", 160, 140, GFXFF);
      if (digitalRead(btn_green) == LOW) {
        if (day_now >= 31) {
          day_now = 1;
        } else {
          day_now = day_now + 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
      if (digitalRead(btn_yellow) == LOW) {
        if (day_now <= 1) {
          day_now = 31;
        } else {
          day_now = day_now - 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
    }
    if (cursorSet_ == 4) {
      tft.drawString("Hour", 160, 140, GFXFF);
      if (digitalRead(btn_green) == LOW) {
        if (hh_now >= 23) {
          hh_now = 0;
        } else {
          hh_now = hh_now + 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
      if (digitalRead(btn_yellow) == LOW) {
        if (hh_now <= 0) {
          hh_now = 23;
        } else {
          hh_now = hh_now - 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
    }
    if (cursorSet_ == 5) {
      tft.drawString("Minute", 160, 140, GFXFF);
      if (digitalRead(btn_green) == LOW) {
        if (mm_now >= 59) {
          mm_now = 0;
        } else {
          mm_now = mm_now + 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
      if (digitalRead(btn_yellow) == LOW) {
        if (mm_now <= 0) {
          mm_now = 59;
        } else {
          mm_now = mm_now - 1;
        }
        tft.fillScreen(TFT_BLACK);
        delay(100);
      }
    }
    if (cursorSet_ == 6) {
      tft.fillScreen(TFT_BLACK);
      cursorSet_ = 0;
      delay(300);
      break;
    }
    date_now = String(year_now) + " - " + String(month_now) + " - " + String(day_now);
    time_now = String(hh_now) + " : " + String(mm_now) + " : 0";
    Serial.println("Date & Time");
    Serial.println("Date : " + String(day_now) + " - " + String(month_now) + " - " + String(year_now));
    Serial.println("Time : " + String(hh_now) + " - " + String(mm_now) + " - 0");
    Serial.print("cursor : ");
    Serial.println(cursorSet_);
    delay(500);
  }
}
void buzzer_salat() {
  DateTime now = rtc.now();
  if (now.hour() == hh_subuh && now.minute() == mm_subuh && now.second() >= 0 && now.second() <= 10) {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(15);
    audio.connecttoFS(SD, "/Setup_Musics/music_subuh.mp3");
  }
  if (now.hour() == hh_dzuhur && now.minute() == mm_dzuhur && now.second() >= 0 && now.second() <= 10) {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(15);
    audio.connecttoFS(SD, "/Setup_Musics/music_dzuhur.mp3");
  }
  if (now.hour() == hh_ashar && now.minute() == mm_ashar && now.second() >= 0 && now.second() <= 10) {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(15);
    audio.connecttoFS(SD, "/Setup_Musics/music_ashar.mp3");
  }
  if (now.hour() == hh_maghrib && now.minute() == mm_maghrib && now.second() >= 0 && now.second() <= 10) {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(15);
    audio.connecttoFS(SD, "/Setup_Musics/music_maghrib.mp3");
  }
  if (now.hour() == hh_isya && now.minute() == mm_isya && now.second() >= 0 && now.second() <= 10) {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(15);
    audio.connecttoFS(SD, "/Setup_Musics/music_isya.mp3");
  }
}
