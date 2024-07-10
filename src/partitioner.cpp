#include "globals.h"
#include "partitioner.h"
#include "display.h"
#include "mykeyboard.h"
#include "esp_heap_caps.h"
#include "sd_functions.h"

// Define o tamanho da partição
#define PARTITION_SIZE 4096

// Using "buff[4096]" to store and write the partitions
#if defined(CARDPUTER) || defined(STICK_C_PLUS2)
const uint8_t def_part[224] PROGMEM = { // default partition scheme(App, FAT and SPIFFS)
    0xAA, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x6E, 0x76, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x18, 0x00, 0x61, 0x70, 0x70, 0x30, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x10, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x61, 0x70, 0x70, 0x31, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x81, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x08, 0x00, 0x76, 0x66, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x82, 0x00, 0x00, 0x6F, 0x00, 0x00, 0x00, 0x10, 0x00, 0x73, 0x70, 0x69, 0x66, 
    0x66, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x03, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x01, 0x00, 0x63, 0x6F, 0x72, 0x65, 
    0x64, 0x75, 0x6D, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xEB, 0xEB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x9C, 0x9E, 0xB3, 0x23, 0x2A, 0x42, 0x20, 0x8E, 0xE9, 0x50, 0xF7, 0xC1, 0x15, 0x7E, 0xEE, 0xED,
};

const uint8_t doom[160] = { // 6Mb app partition
    0xAA, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x6E, 0x76, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x18, 0x00, 0x61, 0x70, 0x70, 0x30, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x10, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x66, 0x00, 0x61, 0x70, 0x70, 0x31, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x03, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x01, 0x00, 0x63, 0x6F, 0x72, 0x65, 
    0x64, 0x75, 0x6D, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xEB, 0xEB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xE0, 0x85, 0x61, 0x59, 0x34, 0xEE, 0xCC, 0xE7, 0x18, 0xD6, 0x6B, 0xEB, 0x0D, 0xA6, 0xEA, 0x41,
};

const uint8_t uiflow2[192] PROGMEM = { // uiflow partition scheme, APP, sys(FAT) and vfs(FAT)
    0xAA, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x6E, 0x76, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x18, 0x00, 0x61, 0x70, 0x70, 0x30, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x10, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x45, 0x00, 0x61, 0x70, 0x70, 0x31, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x81, 0x00, 0x00, 0x5E, 0x00, 0x00, 0x00, 0x10, 0x00, 0x73, 0x79, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x81, 0x00, 0x00, 0x6E, 0x00, 0x00, 0x00, 0x12, 0x00, 0x76, 0x66, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xEB, 0xEB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x48, 0xF8, 0x55, 0x10, 0x56, 0x7F, 0x21, 0xB9, 0x2C, 0x39, 0x4A, 0xA9, 0xF7, 0xC9, 0x22, 0x3B,
};

#elif defined(STICK_C_PLUS)

const uint8_t def_part[192] = { // 4Mb app partition
    0xAA, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x6E, 0x76, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x6F, 0x74, 0x61, 0x64,
    0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x17, 0x00, 0x74, 0x65, 0x73, 0x74, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x10, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x26, 0x00, 0x61, 0x70, 0x70, 0x30, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x82, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x02, 0x00, 0x73, 0x70, 0x69, 0x66, 
    0x66, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xEB, 0xEB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x4B, 0xF5, 0x09, 0xF6, 0xEB, 0x79, 0xF1, 0x66, 0x5B, 0xDC, 0xCF, 0xB3, 0xFF, 0x0E, 0x6B, 0x99,
};

const uint8_t orca[192] = { // 4Mb app partition
    0xAA, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x6E, 0x76, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x6F, 0x74, 0x61, 0x64, 
    0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x17, 0x00, 0x74, 0x65, 0x73, 0x74, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x10, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x20, 0x00, 0x61, 0x70, 0x70, 0x30, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x82, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x08, 0x00, 0x73, 0x70, 0x69, 0x66, 
    0x66, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xEB, 0xEB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xE5, 0x00, 0x17, 0x0A, 0x68, 0x12, 0x14, 0xEA, 0xFE, 0x89, 0x1E, 0x0F, 0x92, 0x04, 0x16, 0x1A,
};
#elif defined(CORE) || defined(CORE2) || defined(CORE3)
  const uint8_t def_part[288] PROGMEM = {
    0xAA, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x6E, 0x76, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x61, 0x70, 0x70, 0x30, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x10, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x80, 0x00, 0x61, 0x70, 0x70, 0x31, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x81, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x10, 0x00, 0x73, 0x79, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x81, 0x00, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x20, 0x00, 0x76, 0x66, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x82, 0x00, 0x00, 0xD0, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x73, 0x70, 0x69, 0x66, 
    0x66, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x03, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x63, 0x6F, 0x72, 0x65, 
    0x64, 0x75, 0x6D, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xEB, 0xEB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x2C, 0x4E, 0x70, 0x13, 0x8D, 0xF3, 0xB0, 0xF7, 0xBF, 0x69, 0x7C, 0xF1, 0x13, 0xDB, 0x36, 0xC1,
  };

  const uint8_t uiFlow1[352] PROGMEM = {
    0xAA, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x6E, 0x76, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x01, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x70, 0x68, 0x79, 0x5F, 
    0x69, 0x6E, 0x69, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1D, 0x00, 0x61, 0x70, 0x70, 0x30, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x00, 0x10, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x61, 0x70, 0x70, 0x31, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x81, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x53, 0x00, 0x76, 0x66, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x40, 0x40, 0x00, 0xE0, 0xBF, 0x00, 0x00, 0x10, 0x00, 0x00, 0x63, 0x6F, 0x6E, 0x66, 
    0x69, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x50, 0x50, 0x00, 0xF0, 0xBF, 0x00, 0x00, 0x10, 0x00, 0x00, 0x77, 0x69, 0x66, 0x69, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x81, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x10, 0x00, 0x73, 0x79, 0x73, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x82, 0x00, 0x00, 0xD0, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x73, 0x70, 0x69, 0x66, 
    0x66, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xAA, 0x50, 0x01, 0x03, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x63, 0x6F, 0x72, 0x65, 
    0x64, 0x75, 0x6D, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xEB, 0xEB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x38, 0x4C, 0x68, 0xD6, 0x6A, 0x40, 0x6E, 0x11, 0xB8, 0x86, 0xC8, 0xA7, 0xBE, 0xD5, 0x72, 0xF9,
  };  
#endif

// Função para apagar e escrever na região de memória flash
bool partitionSetter(const uint8_t* scheme, size_t scheme_size) {
    uint8_t* buffer = (uint8_t*)heap_caps_malloc(PARTITION_SIZE, MALLOC_CAP_INTERNAL);
    if (buffer == NULL) {
        ESP_LOGE("FLASH", "Failed to allocate buffer in DRAM");
        return false;
    }

    // Preencher o buffer com 0xFF
    memset(buffer, 0xFF, PARTITION_SIZE);

    // Copiar o esquema de partição para o buffer
    memcpy(buffer, scheme, scheme_size);

    esp_err_t err;


    // Apagar a região de memória flash
    err = spi_flash_erase_range(0x8000, PARTITION_SIZE);
    if (err != ESP_OK) {
        ESP_LOGE("FLASH", "Failed to erase flash region (0x%x)", err);
        heap_caps_free(buffer);
        return false;
    }

    // Escrever o buffer na memória flash
    err = esp_flash_write(NULL, buffer, 0x8000, PARTITION_SIZE);
    if (err != ESP_OK) {
        ESP_LOGE("FLASH", "Failed to write to flash (0x%x)", err);
        heap_caps_free(buffer);
        return false;
    }

    heap_caps_free(buffer);
    return true;
}

void partitioner() {
    int partition = 100;
    const uint8_t* data = nullptr;
    size_t data_size = 0;

    // Opções de partição
    options = {
        {"Default", [&](){ partition = 0; }},
      #if defined(CARDPUTER) || defined(STICK_C_PLUS2)  
        {"Doom", [&](){ partition = 1; }},
        {"UiFlow2", [&](){ partition = 2; }},
      #elif defined(STICK_C_PLUS)
        {"Orca", [&](){ partition = 1; }},
      #elif defined(CORE) || defined(CORE2) || defined(CORE3)
        {"UiFlow1", [&](){ partition = 1; }},
      #endif      
    };
    delay(200);
    loopOptions(options);

    if(partition==100) goto Exit;
    switch(partition) {
        case 0: data = def_part;
                data_size = sizeof(def_part);
                break;
      #if defined(CARDPUTER) || defined(STICK_C_PLUS2)
        case 1: data = doom;
                data_size = sizeof(doom);
                break;
        case 2: data = uiflow2;
                data_size = sizeof(uiflow2);
                break;
      #elif defined(STICK_C_PLUS)
        case 1: data = orca;
                data_size = sizeof(orca);
                break;
      #elif defined(CORE) || defined(CORE2) || defined(CORE3)
        case 1: data = uiFlow1;
                displayRedStripe("Experimental");
                delay(2000);
                data_size = sizeof(uiFlow1);
                break;                
      #endif
        default: data = def_part;
                data_size = sizeof(def_part);
    }

    if (!partitionSetter(data, data_size)) {
        Serial.println("Error when running partitionSetter function");
        displayRedStripe("Partitioning Error");
        while (!checkSelPress()) yield();
    }

    displayRedStripe("Restart needed");

    while (!checkSelPress()) yield();
    while (checkSelPress()) yield();
    ESP.restart();
Exit:
    Serial.print("Desistiu");
}

void partList() {
      // Obtemos a lista de partições
  const esp_partition_t* partition;
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if (it != NULL) {
    Serial.println("Partições encontradas:");
    String txt="";
    int i=0;
    while (it != NULL) {
      partition = esp_partition_get(it);

      switch (partition->subtype) {
        case ESP_PARTITION_SUBTYPE_APP_OTA_0:
        case ESP_PARTITION_SUBTYPE_APP_OTA_1:
          Serial.println("OTA");
          txt+="-OTA-";
          break;

        case ESP_PARTITION_SUBTYPE_DATA_FAT:
          Serial.println("FAT");
          txt+="FAT-";
          break;
        case ESP_PARTITION_SUBTYPE_DATA_SPIFFS:
          Serial.println("SPIFFS");
          txt+="SPIFFs-";
          break;
        default:
          Serial.println("Desconhecido");
          break;
      }    
      it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);
    displayRedStripe(txt);
    delay(300);
    while(!checkSelPress()) yield();
    while(checkSelPress()) yield();    
  }   
}


void dumpPartition(const char* partitionLabel, const char* outputPath) {
  tft.fillScreen(BGCOLOR);
  const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, partitionLabel);
  if (partition == NULL) {
    Serial.printf("Partição %s não encontrada\n", partitionLabel);
    return;
  }

  setupSdCard();
  if (!SD.exists("/bkp")) SD.mkdir("/bkp");

  File outputFile = SD.open(outputPath, FILE_WRITE);
  if (!outputFile) {
    Serial.printf("Falha ao abrir o arquivo %s no cartão SD\n", outputPath);
    return;
  }

  Serial.printf("Iniciando dump da partição %s para o arquivo %s\n", partitionLabel, outputPath);

  const size_t bufferSize = 1024;  // Ajuste conforme necessário
  uint8_t buffer[1024];
  size_t bytesToRead = 0;
  esp_err_t result;
  progressHandler(0,500);
  displayRedStripe("Backing up");
  for (size_t offset = 0; offset < partition->size; offset += bufferSize) {
    bytesToRead = (offset + bufferSize > partition->size) ? (partition->size - offset) : bufferSize;
    result = esp_partition_read(partition, offset, buffer, bytesToRead);
    if (result != ESP_OK) {
      Serial.printf("Erro ao ler a partição %s no offset %d (código de erro: %d)\n", partitionLabel, offset, result);
      outputFile.close();
      return;
    }
    outputFile.write(buffer, bytesToRead);
    progressHandler(int(offset+bufferSize),partition->size);
  }
  outputFile.close();
  displayRedStripe("    Complete!    ");
  delay(2000);

  Serial.printf("Dump da partição %s para o arquivo %s concluído\n", partitionLabel, outputPath);
}

void restorePartition(const char* partitionLabel) {
  String filepath = loopSD(true);
  tft.fillScreen(BGCOLOR);
  if(filepath=="") return;
  else {
    File source = SD.open(filepath, "r");
    if(strcmp(partitionLabel,"spiffs")==0) {
      prog_handler=1;
      Update.begin(source.size(),U_SPIFFS);
      uint8_t buffer[1024];
      int bytesRead = 0;
      int written = 0;
      size_t total = source.size();
      progressHandler(0,500);
      while (source.available()) {
        bytesRead = source.read(buffer, sizeof(buffer));
        Update.write(buffer,bytesRead);
        written+=bytesRead;
        progressHandler(written, total);
      }
    }

    if(strcmp(partitionLabel,"vfs")==0) {
      performFATUpdate(source,source.size(), "vfs");
    }
    if(strcmp(partitionLabel,"sys")==0) {
      performFATUpdate(source,source.size(), "sys");
    }
  }
  delay(100);
  displayRedStripe("    Restored!    ");
  delay(2000);
}


#define TAG "Partitioneer"
#define BUFFER_SIZE 1024

// Função para copiar partições com buffer de 1024 bytes
esp_err_t copy_partition(const esp_partition_t* src, const esp_partition_t* dst) {
    uint8_t buffer[BUFFER_SIZE];
    esp_err_t err;
    progressHandler(0,500);
    displayRedStripe("Launcher Update");
    for (size_t offset = 0; offset < dst->size; offset += BUFFER_SIZE) {
        size_t read_size = BUFFER_SIZE;
        if (offset + BUFFER_SIZE > dst->size) {
            read_size = dst->size - offset;
        }

        err = esp_partition_read(src, offset, buffer, read_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read source partition at offset %u", offset);
            return err;
        }

        err = esp_partition_write(dst, offset, buffer, read_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write to destination partition at offset %u", offset);
            return err;
        }
        progressHandler(offset+BUFFER_SIZE,dst->size);
    }

    return ESP_OK;
}

// Função principal
void partitionCrawler() {
    const esp_partition_t* running_partition = esp_ota_get_running_partition();
    if (running_partition == NULL) {
        ESP_LOGE(TAG, "Failed to get running partition");
        return;
    }

    if (running_partition->subtype == ESP_PARTITION_SUBTYPE_APP_TEST) {
        ESP_LOGI(TAG, "Running partition is ESP_PARTITION_SUBTYPE_APP_TEST, no action taken");
        return;
    }

    displayRedStripe("Updating...");

    const esp_partition_t* test_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_TEST, NULL);

    if (test_partition == NULL) {
        ESP_LOGE(TAG, "Failed to find test partition");
        return;
    }

    ESP_LOGI(TAG, "Erasing test partition");
    esp_err_t err = esp_partition_erase_range(test_partition, 0, test_partition->size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase test partition");
        return;
    }

    ESP_LOGI(TAG, "Copying running partition to test partition");
    err = copy_partition(running_partition, test_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to copy partition data");
        displayRedStripe("Use M5Burner!");
        delay(10000);
        return;
    }

    ESP_LOGI(TAG, "Writing 0x00 to first byte of the running partition");
    uint8_t zero_byte = 0x00;
    err = esp_partition_write(running_partition, 0, &zero_byte, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write 0x00 to the first byte of the running partition");
    } else {
        ESP_LOGI(TAG, "Restarting system to boot from test partition");
        esp_restart();
    }
}