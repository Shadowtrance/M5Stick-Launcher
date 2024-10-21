#include "globals.h"

#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <M5-HTTPUpdate.h>
#ifdef HEADLESS
#include "util/VectorDisplay.h"
#else
#include <TFT_eSPI.h>
#endif
#include <SPIFFS.h>
#include "esp_ota_ops.h"

#include <iostream>
#include <functional>
#include <vector>
#include <string>


// Public Globals
uint32_t MAX_SPIFFS = 0;
uint32_t MAX_APP = 0;
uint32_t MAX_FAT_vfs = 0;
uint32_t MAX_FAT_sys = 0;
uint16_t FGCOLOR = TFT_GREEN;
uint16_t ALCOLOR = TFT_RED;
uint16_t BGCOLOR = TFT_BLACK;
uint16_t odd_color = 0x30c5;
uint16_t even_color = 0x32e5;

#if defined(HEADLESS)
uint8_t _miso=0;
uint8_t _mosi=0;
uint8_t _sck=0;
uint8_t _cs=0;
#endif

unsigned long dimmerTemp=millis();
int dimmerSet=20;
int bright=100;
bool dimmer=false;
int prog_handler;    // 0 - Flash, 1 - SPIFFS
int currentIndex;
int rotation;
bool sdcardMounted;
bool onlyBins;
bool returnToMenu;
bool update;
bool askSpiffs;
bool stopOta;
//bool command;
size_t file_size;
String ssid;
String pwd;
String wui_usr = "admin";
String wui_pwd = "launcher";
String dwn_path = "/downloads/";
JsonDocument doc;
JsonDocument settings;
std::vector<std::pair<std::string, std::function<void()>>> options;
const int bufSize = 1024;
uint8_t buff[1024] = {0};

#include "display.h"
#include "mykeyboard.h"
#include "onlineLauncher.h"
#include "sd_functions.h"
#include "webInterface.h"
#include "partitioner.h"
#include "settings.h"




/*********************************************************************
**  Function: get_partition_sizes                                    
**  Get the size of the partitions to be used when installing
*********************************************************************/
void get_partition_sizes() {
    // Obter a tabela de partições
    const esp_partition_t* partition;
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);

    // Iterar sobre as partições do tipo APP
    while (it != NULL) {
        partition = esp_partition_get(it);
        if (partition != NULL && partition->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0) {
            MAX_APP = partition->size;
        }
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);

    // Iterar sobre as partições do tipo DATA
    it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
    while (it != NULL) {
        partition = esp_partition_get(it);
        if (partition != NULL) {
            if (partition->subtype == ESP_PARTITION_SUBTYPE_DATA_SPIFFS) {
                MAX_SPIFFS = partition->size;
            } else if (partition->subtype == ESP_PARTITION_SUBTYPE_DATA_FAT) {
              log_i("label: %s",partition->label);
              if(strcmp(partition->label,"vfs")==0) MAX_FAT_vfs = partition->size;
              else if (strcmp(partition->label,"sys")==0) MAX_FAT_sys = partition->size;
            }
        }
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);
    if(MAX_SPIFFS==0 && askSpiffs) gsetAskSpiffs(true, false);

    // Logar os tamanhos das partições
    ESP_LOGI("Partition Sizes", "MAX_APP: %d", MAX_APP);
    ESP_LOGI("Partition Sizes", "MAX_SPIFFS: %d", MAX_SPIFFS);
    ESP_LOGI("Partition Sizes", "MAX_FAT_sys: %d", MAX_FAT_sys);
    ESP_LOGI("Partition Sizes", "MAX_FAT_vfs: %d", MAX_FAT_vfs);

#if defined(HEADLESS)
    // This piece of the code is supposed to handle the absence of APP partition tables
    // if it doent's find App partition, means that the ESP32 can be 4, 8 or 16Mb, that must be set in platformio.ini
    // and the partition size will be discovered using esp_flash_get_physical_size function
    uint32_t __size;  // Variable to store the flash size from flash ID
    //esp_err_t result = esp_flash_get_physical_size(NULL,&__size); // This guy reads the FlashID size, won't work for now, that is the true size of the chip
                                                                    // if flashing with a 4Mb bootloader and installing 8Mb partition scheme, it breakes (bootloop) probably in this function:
                                                                    // https://github.com/bmorcelli/esp-idf/blob/621a7fa1208ce44dbf1e0038c43587a1dc362319/components/spi_flash/esp_flash_spi_init.c#L282
                                                                    // If i set a chip size greater than the one I have, I fall in this error:
                                                                    // https://github.com/bmorcelli/esp-idf/blob/621a7fa1208ce44dbf1e0038c43587a1dc362319/components/spi_flash/esp_flash_spi_init.c#L317
                                                                    // maybe commenting it in a custom fw can solve it, but will add lots of issues due to size changings and sets in this same function
                                                                    
    esp_err_t result = esp_flash_get_size(NULL,&__size);
    if (result == ESP_OK) {
        Serial.print("Flash size: ");
        Serial.println(__size, HEX);  // Prints value in hex
    } else {
        Serial.print("Error: ");
        Serial.println(result);  // Prints the errors, if any
    }
    if(MAX_APP==0) {
      // Makes the arrangements

      //partitionSetter(def_part, sizeof(def_part));
      if(__size==0x400000) { partitionSetter(def_part, sizeof(def_part)); }
      if(__size==0x800000) { partitionSetter(def_part8, sizeof(def_part8));}
      if(__size==0x1000000) { partitionSetter(def_part16, sizeof(def_part16)); }
      while(1) {
        Serial.println("Turn Off and On again to apply partition changes.");
        delay(2000);
      }
    }

  #endif
}

/*********************************************************************
**  Function: setup                                    
**  Where the devices are started and variables set    
*********************************************************************/
void setup() {
  Serial.begin(115200);

  // Setup GPIOs and stuff
  #if defined(STICK_C_PLUS2)
    pinMode(UP_BTN, INPUT);
    pinMode(SEL_BTN, INPUT);
    pinMode(DW_BTN, INPUT);
    pinMode(4, OUTPUT);
    digitalWrite(4,HIGH);
    //Grove pins down
    pinMode(32, OUTPUT);
    pinMode(33, OUTPUT);
    digitalWrite(32,LOW);
    digitalWrite(33,HIGH); //CS pin for CC1101 bruce pin

  #elif defined(STICK_C_PLUS)
    axp192.begin();
    pinMode(SEL_BTN, INPUT);
    pinMode(DW_BTN, INPUT);
    //Grove Pins down
    pinMode(32, OUTPUT);
    pinMode(33, OUTPUT);
    digitalWrite(32,LOW);
    digitalWrite(33,HIGH);//CS pin for CC1101 bruce pin
  #elif defined(M5STACK) && defined(CORE3)
    M5.begin(); // Begin after TFT, for SDCard to work
  #elif defined(CARDPUTER)
    Keyboard.begin();
    pinMode(10, INPUT);
    //Grove Pins down
    pinMode(1, OUTPUT);
    pinMode(2, OUTPUT);
    digitalWrite(1,HIGH);//CS pin for CC1101 bruce pin
    digitalWrite(2,LOW);
  #elif defined(T_DISPLAY_S3)
    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
    gpio_hold_dis((gpio_num_t)21);//PIN_TOUCH_RES 
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);//PIN_POWER_ON 
    pinMode(21, OUTPUT); //PIN_TOUCH_RES 
    digitalWrite(21, LOW);//PIN_TOUCH_RES 
    delay(500);
    digitalWrite(21, HIGH);//PIN_TOUCH_RES 
    Wire.begin(18, 17);//SDA, SCL
    if (!touch.init()) {
        Serial.println("Touch IC not found");
    }
  #elif defined(CYD)
  pinMode(XPT2046_CS, OUTPUT);
  //touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  if(!touch.begin()) {
      Serial.println("Touch IC not Started");
      log_i("Touch IC not Started");
  } else log_i("Touch IC Started");
  digitalWrite(XPT2046_CS, LOW);


  #elif defined(T_EMBED)
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
  #ifdef T_EMBED_1101
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);  // Power on CC1101 and LED
    PPM.init(Wire,8,18,BQ25896_SLAVE_ADDRESS);
    // Set the minimum operating voltage. Below this voltage, the PPM will protect
    PPM.setSysPowerDownVoltage(3300);
    // Set input current limit, default is 500mA
    PPM.setInputCurrentLimit(3250);
    // Disable current limit pin
    PPM.disableCurrentLimitPin();
    // Set the charging target voltage, Range:3840 ~ 4608mV ,step:16 mV
    PPM.setChargeTargetVoltage(4208);
    // Set the precharge current , Range: 64mA ~ 1024mA ,step:64mA
    PPM.setPrechargeCurr(64);
    // The premise is that Limit Pin is disabled, or it will only follow the maximum charging current set by Limi tPin.
    // Set the charging current , Range:0~5056mA ,step:64mA
    PPM.setChargerConstantCurr(832);
    // Get the set charging current
    PPM.getChargerConstantCurr();
    Serial.printf("getChargerConstantCurr: %d mA\n",PPM.getChargerConstantCurr());


    // To obtain voltage data, the ADC must be enabled first
    PPM.enableADCMeasure();
    
    // Turn on charging function
    // If there is no battery connected, do not turn on the charging function
    PPM.enableCharge();
    pinMode(12, OUTPUT);
    digitalWrite(12,HIGH);//CS pin for CC1101 pin
  #else
    pinMode(BAT_PIN,INPUT); // Battery value

  #endif
  pinMode(BK_BTN, INPUT);
  pinMode(ENCODER_KEY, INPUT);
  // use TWO03 mode when PIN_IN1, PIN_IN2 signals are both LOW or HIGH in latch position.
  encoder = new RotaryEncoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

  // register interrupt routine
  attachInterrupt(digitalPinToInterrupt(ENCODER_INA), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_INB), checkPosition, CHANGE);

  #elif defined(T_DECK)
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  pinMode(SEL_BTN, INPUT);

  // Setup for Trackball
  pinMode(UP_BTN, INPUT_PULLUP);
  attachInterrupt(UP_BTN, ISR_up, FALLING);
  pinMode(DW_BTN, INPUT_PULLUP);
  attachInterrupt(DW_BTN, ISR_down, FALLING);
  pinMode(L_BTN, INPUT_PULLUP);
  attachInterrupt(L_BTN, ISR_left, FALLING);
  pinMode(R_BTN, INPUT_PULLUP);
  attachInterrupt(R_BTN, ISR_right, FALLING);
  //pinMode(BACKLIGHT, OUTPUT);
  //digitalWrite(BACKLIGHT,HIGH);

    // PWM backlight setup
  // ledcSetup(TFT_BRIGHT_CHANNEL,TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits); //Channel 0, 10khz, 8bits
  // ledcAttachPin(TFT_BL, TFT_BRIGHT_CHANNEL);
  // ledcWrite(TFT_BRIGHT_CHANNEL,125);

  #elif defined(HEADLESS)
    #if SEL_BTN >= 0 // handle enter in launcher
      pinMode(SEL_BTN, INPUT);
    #endif
    #if LED>0
      pinMode(LED,OUTPUT); // Set pin to to recognize if launcher is starting or not, and connectiong or not
      digitalWrite(LED, LED_ON); // keeps on until exit
    #endif

  #elif HAS_BTN>0
    #if UP_BTN >= 0
    pinMode(UP_BTN, INPUT);
    #endif
    #if SEL_BTN >= 0
    pinMode(SEL_BTN, INPUT);
    #endif
    #if DW_BTN >= 0
    pinMode(DW_BTN, INPUT);
    #endif
  #endif
  #if defined(BACKLIGHT)
  pinMode(BACKLIGHT, OUTPUT);
  #endif

  EEPROM.begin(EEPROMSIZE+32); // open eeprom.... 32 is the size of the SSID string stored at the end of the memory, using this trick to not change all the addresses
  if(EEPROM.read(EEPROMSIZE-13) > 3 || EEPROM.read(EEPROMSIZE-14) > 240 || EEPROM.read(EEPROMSIZE-15) > 100 || EEPROM.read(EEPROMSIZE-1) > 1 || EEPROM.read(EEPROMSIZE-2) > 1 || (EEPROM.read(EEPROMSIZE-3)==0xFF && EEPROM.read(EEPROMSIZE-4) == 0xFF && EEPROM.read(EEPROMSIZE-5)==0xFF && EEPROM.read(EEPROMSIZE-6) == 0xFF)) {
    log_i("EEPROM back to default\n0=%d\n1=%d\n2=%d\n9=%d\nES-1=%d",EEPROM.read(EEPROMSIZE-13),EEPROM.read(EEPROMSIZE-14),EEPROM.read(EEPROMSIZE-15),EEPROM.read(EEPROMSIZE-1),EEPROM.read(EEPROMSIZE-2) );
  #if defined(CARDPUTER) || defined(M5STACK)
    EEPROM.write(EEPROMSIZE-13, 1);    // Right rotation for cardputer
  #else
    EEPROM.write(EEPROMSIZE-13, 3);    // Left rotation
  #endif
    EEPROM.write(EEPROMSIZE-14, 20);  // 20s Dimm time
    EEPROM.write(EEPROMSIZE-15, 100);  // 100% brightness
    EEPROM.write(EEPROMSIZE-1, 1);    // OnlyBins
    EEPROM.writeString(20,"");
    EEPROM.writeString(EEPROMSIZE,""); // resets ssid at the end of the EEPROM
    EEPROM.write(EEPROMSIZE-2, 1);  // AskSpiffs

    //FGCOLOR
    EEPROM.write(EEPROMSIZE-3, 0x07); 
    EEPROM.write(EEPROMSIZE-4, 0xE0);
    //BGCOLOR
    EEPROM.write(EEPROMSIZE-5, 0);
    EEPROM.write(EEPROMSIZE-6, 0);
    //ALCOLOR
    EEPROM.write(EEPROMSIZE-7, 0xF8);
    EEPROM.write(EEPROMSIZE-8, 0x00);
    //odd
    EEPROM.write(EEPROMSIZE-9, 0x30);
    EEPROM.write(EEPROMSIZE-10, 0xC5);
    //even
    EEPROM.write(EEPROMSIZE-11, 0x32);
    EEPROM.write(EEPROMSIZE-12, 0xe5);

  #if defined(HEADLESS)
    // SD Pins
    EEPROM.write(90, 0);
    EEPROM.write(91, 0);
    EEPROM.write(92, 0);
    EEPROM.write(93, 0);
  #endif
    EEPROM.commit();       // Store data to EEPROM
  }
  #if ROTATION == 0 // Marauder mini
  if(EEPROM.read(EEPROMSIZE-13) != 0 && EEPROM.read(EEPROMSIZE-13) != 2)  { 
    EEPROM.write(EEPROMSIZE-13, ROTATION);    // Left rotation
    EEPROM.commit();       // Store data to EEPROM
  }
  #else
  if(EEPROM.read(EEPROMSIZE-13) != 1 && EEPROM.read(EEPROMSIZE-13) != 3)  { 
    EEPROM.write(EEPROMSIZE-13, ROTATION);    // Left rotation
    EEPROM.commit();       // Store data to EEPROM
  }
  #endif
  rotation = EEPROM.read(EEPROMSIZE-13);
  dimmerSet = EEPROM.read(EEPROMSIZE-14);
  bright = EEPROM.read(EEPROMSIZE-15);
  onlyBins = EEPROM.read(EEPROMSIZE-1);
  askSpiffs = EEPROM.read(EEPROMSIZE-2);
  FGCOLOR =    (EEPROM.read(EEPROMSIZE-3)  << 8) | EEPROM.read(EEPROMSIZE-4);
  BGCOLOR =    (EEPROM.read(EEPROMSIZE-5)  << 8) | EEPROM.read(EEPROMSIZE-6);
  ALCOLOR =    (EEPROM.read(EEPROMSIZE-7)  << 8) | EEPROM.read(EEPROMSIZE-8);
  odd_color =  (EEPROM.read(EEPROMSIZE-9)  << 8) | EEPROM.read(EEPROMSIZE-10);
  even_color = (EEPROM.read(EEPROMSIZE-11) << 8) | EEPROM.read(EEPROMSIZE-12);
  pwd = EEPROM.readString(20); // read what is on EEPROM here for headless environment
  ssid = EEPROM.readString(EEPROMSIZE); // read what is on EEPROM here for headless environment

  #if defined(HEADLESS)
    // SD Pins
    _miso=EEPROM.read(90);
    _mosi=EEPROM.read(91);
    _sck= EEPROM.read(92);
    _cs=  EEPROM.read(93);
  #endif

  EEPROM.end();  

  // declare variables
  size_t currentIndex=0;  
  prog_handler=0;
  sdcardMounted=false;
  String fileToCopy;

  //Init Display
  #if !defined(M5STACK)
    #ifndef HEADLESS
      tft.setAttribute(PSRAM_ENABLE,true);
      tft.init();
    #endif
  #endif
  tft.setRotation(rotation);
  tft.fillScreen(BGCOLOR);
  //setBrightness(bright,false);
  initDisplay(true);  

  #if defined(T_DECK)
  delay(500); // time to ESP32C3 start and enable the keyboard
  Wire.begin(KB_I2C_SDA, KB_I2C_SCL);
  #endif


  #if defined(MARAUDERV4)    

 //uint16_t calData[5] = { 275, 3494, 361, 3528, 4 }; //org portrait
uint16_t calData[5] = { 391, 3491, 266, 3505, 7 }; // Landscape TFT Shield from maruader
//uint16_t calData[5] = { 213, 3469, 320, 3446, 1 }; // Landscape TFT DIY  from maruader

    tft.setTouch(calData);
  #endif

#if defined(T_DISPLAY_S3) || defined(CYD)
  touch.setRotation(1);
    // PWM backlight setup
  ledcSetup(TFT_BRIGHT_CHANNEL,TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits); //Channel 0, 10khz, 8bits
  ledcAttachPin(TFT_BL, TFT_BRIGHT_CHANNEL);
  ledcWrite(TFT_BRIGHT_CHANNEL,255);
#endif


#if defined(M5STACK) && !defined(CORE3)
    M5.begin(); // Begin after TFT, for SDCard to work
#endif
  // Performs the verification when Launcher is installed through OTA
  partitionCrawler(); 
  // Checks the size of partitions and take actions to find the best options (in HEADLESS environment)
  get_partition_sizes();
  // Checks if the fw in the OTA partition is valid. reading the firstByte looking for 0xE9
  const esp_partition_t* ota_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
  uint8_t firstByte;
  esp_partition_read(ota_partition,0,&firstByte,1);
  //Gets the config.conf from SD Card and fill out the settings JSON
  getConfigs();
  #if defined(M5STACK)
  coreFooter2();
  #endif    
  #if defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4)
  TdisplayS3Footer2();
  #endif  
  //Start Bootscreen timer
  int i = millis();
  int j = 0;
  while(millis()<i+5000) { // increased from 2500 to 5000
    initDisplay();        //Inicia o display 
    
    if(millis()>(i+j*500)) { // Serial message each ~500ms
      Serial.println("Press the button to enter the Launcher!");
      j++;
    }
  
      if(checkSelPress())    
      {
          tft.fillScreen(BGCOLOR);
          goto Launcher;
        }

    #if defined (CARDPUTER)
      Keyboard.update();
      if (Keyboard.isPressed() && !(Keyboard.isKeyPressed(KEY_ENTER)))
    #elif defined(STICK_C_PLUS2) || defined(STICK_C_PLUS)
      if(checkNextPress()) 
    #else
      if(checkNextPress() || checkPrevPress())
    #endif 
      {
        tft.fillScreen(TFT_BLACK);
        ESP.restart();
      } 
  }
  
  // If nothing is done, check if there are any app installed in the ota partition, if it does, restart device to start installed App.
  if(firstByte==0xE9) {
    tft.fillScreen(TFT_BLACK);
	  ESP.restart();  
  }
  else goto Launcher;


  // If M5 or Enter button is pressed, continue from here
  Launcher:
  
  tft.fillScreen(BGCOLOR);
  #if REDRAW_DELAY<50 // Encoder devices need delay to not open the firt option in the menu
  delay(200);
  #endif
  #if LED>0 && defined(HEADLESS)
    digitalWrite(LED, LED_ON?LOW:HIGH); // turn off the LED
  #endif
}

/**********************************************************************
**  Function: loop                                     
**  Main loop                                          
**********************************************************************/
#ifndef HEADLESS
void loop() {
  bool redraw = true;
  int index = 0;
  int opt = 4; // there are 3 options> 1 list SD files, 2 OTA and 3 Config
  stopOta = false; // variable used in WebUI, and to prevent open OTA after webUI without restart
  getBrightness();
  if(!sdcardMounted) index=1; //if SD card is not present, paint SD square grey and auto select OTA
  while(1){
    if (redraw) { 
      drawMainMenu(index); 
      #if defined(M5STACK)
      coreFooter();
      #elif defined(T_DISPLAY_S3) || defined(CYD) || defined(MARAUDERV4)
      TdisplayS3Footer();
      #endif      
      redraw = false; 
      delay(REDRAW_DELAY); 
    }

    if(checkPrevPress()) {
      if(index==0) index = opt - 1;
      else if(index>0) index--;
      redraw = true;
    }
    // DW Btn to next item 
    if(checkNextPress()) { 
      index++;
      if((index+1)>opt) index = 0;
      redraw = true;
    }

    // Select and run function 
    if(checkSelPress()) { 
      if(index == 0) {  
        if(setupSdCard()) { 
          loopSD(false); 
          tft.fillScreen(BGCOLOR);
          redraw=true;
        }
        else {
          displayRedStripe("Insert SD Card");
          delay(3000);
        }
        
      }
      if(index == 1) {  
        if (!stopOta) {
          if (WiFi.status() != WL_CONNECTED) {
            int nets;
            WiFi.mode(WIFI_MODE_STA);
            displayRedStripe("Scanning...");
            nets=WiFi.scanNetworks();
            //delay(3000);
            options = { };
            for(int i=0; i<nets; i++){
              options.push_back({WiFi.SSID(i).c_str(), [=]() { wifiConnect(WiFi.SSID(i).c_str(),int(WiFi.encryptionType(i))); }});
            }
            delay(200);
            loopOptions(options);
            if (WiFi.status() == WL_CONNECTED) {
              if(GetJsonFromM5()) loopFirmware();
            }

          } else {
            //If it is already connected, download the JSON again... it loses the information once you step out of loopFirmware(), dkw
            closeSdCard();
            if(GetJsonFromM5()) loopFirmware();
          }
          tft.fillScreen(BGCOLOR);
          redraw=true;
        } 
        else {
          displayRedStripe("Restart to open OTA");
          delay(3000);
        } 

      }
      if(index == 2) {
        loopOptionsWebUi();
        tft.fillScreen(BGCOLOR);
        redraw=true;        
      }

      if(index == 3) {  
        options = {
          
          {"Charge Mode", [=](){ chargeMode(); }},
          {"Brightness", [=]() { setBrightnessMenu(); }},
          {"Dim time", [=]()   { setdimmerSet();}},
          {"UI Color", [=]()   { setUiColor();}},
        };
        if(sdcardMounted) {
          if(onlyBins) options.push_back({"All Files",  [=]() { gsetOnlyBins(true, false); saveConfigs();}});
          else         options.push_back({"Only Bins",  [=]() { gsetOnlyBins(true, true); saveConfigs();}});
        }
        
        if(askSpiffs) options.push_back({"Avoid Spiffs",  [=]() { gsetAskSpiffs(true, false); saveConfigs();}});
        else          options.push_back({"Ask Spiffs",    [=]() { gsetAskSpiffs(true, true); saveConfigs();}});
        options.push_back({"Rotate 180",  [=]() { gsetRotation(true); }});
        options.push_back({"Part Change",  [=]() { partitioner(); }});
        options.push_back({"Part List",  [=]() { partList(); }});

      #ifndef STICK_C_PLUS
        options.push_back({"Clear FAT",  [=]() { eraseFAT(); }});
      #endif

        if(MAX_SPIFFS>0) options.push_back({"Save SPIFFS",  [=]() { dumpPartition("spiffs", "/bkp/spiffs.bin"); }});
        //if(MAX_FAT_sys>0) options.push_back({"Bkp FAT sys",  [=]() { dumpPartition("sys", "/bkp/FAT_sys.bin"); }});    //Test only
        if(MAX_FAT_vfs>0) options.push_back({"Save FAT vfs",  [=]() { dumpPartition("vfs", "/bkp/FAT_vfs.bin"); }});
        if(MAX_SPIFFS>0) options.push_back({ "Rest SPIFFS",  [=]() { restorePartition("spiffs"); }});
        //if(MAX_FAT_sys>0) options.push_back({"Rest FAT Sys",  [=]() { restorePartition("sys"); }});                     //Test only
        if(MAX_FAT_vfs>0) options.push_back({"Rest FAT Vfs",  [=]() { restorePartition("vfs"); }});

        options.push_back({"Restart",  [=]() { ESP.restart(); }});

        #if defined(STICK_C_PLUS2)
        options.push_back({"Turn-off",  [=]() { digitalWrite(4,LOW); }});
        #elif defined(T_EMBED_1101)
        options.push_back({"Turn-off",  [=]() { digitalWrite(PIN_POWER_ON,LOW); esp_sleep_enable_ext0_wakeup(GPIO_NUM_6,LOW); esp_deep_sleep_start(); }});
        #elif defined(T_EMBED)
        options.push_back({"Turn-off",  [=]() { digitalWrite(PIN_POWER_ON,LOW); }});
        #elif defined(STICK_C_PLUS)
        options.push_back({"Turn-off",  [=]() { axp192.PowerOff(); }});
        #endif
        options.push_back({"Main Menu",  [=]() { returnToMenu=true; }});
        delay(200);
        loopOptions(options);
        tft.fillScreen(BGCOLOR);
        tft.fillScreen(BGCOLOR);
        redraw=true;
      }
      returnToMenu = false;
      redraw = true;
    }
  }
}

#else
void loop(){
  // Start SD card, If there's no SD Card installed, see if there's ssid saved on memory, 
  Serial.print(
    "     _                            _               \n"
    "    | |                          | |              \n"
    "    | |     __ _ _   _ _ __   ___| |__   ___ _ __ \n"
    "    | |    / _` | | | | '_ \\ / __| '_ \\ / _ \\ '__|\n"
    "    | |___| (_| | |_| | | | | (__| | | |  __/ |   \n"
    "    |______\\__,_|\\__,_|_| |_|\\___|_| |_|\\___|_|   \n"
    "    ----------------------------------------------\n"
                                                
                                              
    "Welcome to Launcher, an ESP32 firmware where you can have\n"
    "a better control on what you are running on it.\n\n"
    "Now it will Start a web interface, where you can flash a new\n"
    "firmware on a dedicated partition, and swap it whenever you\n"
    "want using this Launcher.\n\n\n"
  );  
  
  getConfigs();
  Serial.println("Scanning networks...");
  int nets = WiFi.scanNetworks();
  bool mode_ap=true;
  
  if(sdcardMounted) {
    JsonObject setting = settings[0];
    JsonArray WifiList = setting["wifi"].as<JsonArray>();
    for(int i=0; i<nets; i++) {
      for(auto wifientry : WifiList) {
        Serial.println("Target: " + ssid + " Network: " + WiFi.SSID(i));
        if(WiFi.SSID(i)==wifientry["ssid"].as<String>()) {
          ssid = wifientry["ssid"].as<String>();
          pwd = wifientry["pwd"].as<String>();
          WiFi.begin(ssid, pwd);
          int count = 0;
          Serial.println("Connecting to " + ssid);
          while (WiFi.status() != WL_CONNECTED) {
            delay(500);
          #if LED>0
            digitalWrite(LED, count&1?LED_ON:(LED_ON?LOW:HIGH)); // blink the LED
          #endif
            Serial.print(".");
            count++;
            if (count > 10) { // try for 5 seconds
              break; // stops trying this network, will try the others, if there are some other with same SSID
            }
          }
          if(WiFi.status() != WL_CONNECTED) {
            // saves last connected network into EEPROM
            EEPROM.begin(EEPROMSIZE+32);
            EEPROM.writeString(20,pwd);
            EEPROM.writeString(EEPROMSIZE,ssid);
            EEPROM.end();
          }
        }
      }
    }
  }
  else if(ssid!="") { // will try to connect to a saved network
    for(int i=0; i<nets; i++) {
      Serial.println("Target: " + ssid + " Network: " + WiFi.SSID(i));
      if(ssid == WiFi.SSID(i)) {
        Serial.println("Network matches the SSID, starting connection\n");
        WiFi.begin(ssid, pwd);
        int count = 0;
        Serial.println("Connecting to " + ssid);
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
        #if LED>0
          digitalWrite(LED, count&1?LED_ON:(LED_ON?LOW:HIGH)); // blink the LED
        #endif
          Serial.print(".");
          count++;
          if (count > 6) { // try for 3 seconds
            break; // stops trying this network, will try the others, if there are some other with same SSID, it can take quite sometime :/
          }
        }
      }
    }
  }
  else {
    Serial.println(
      "Couldn't find SD Card and SSID Saved,\n"
      "you can configure it on the WEB Ui,\n\n"
      "Starting the Launcher in Access point mode\n"
      "Connect into the following network\n"
      "with no other network (mobile data off and unplug wired connections)"
    );
  }

  // if there's no network information, open in Access Point Mode
  if(WiFi.status() == WL_CONNECTED) mode_ap=false;

  startWebUi("", 0, mode_ap);

  // sorfware will keep trapped in startWebUi loop..


}
#endif