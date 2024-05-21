#include "Memoria.h"
#include "Arduino.h"

#include <FS.h>
#include <LittleFS.h>

#include <SD_MMC.h>

using Condicion = CondicionSimple;

Memoria::Memoria(int clk, int cmd, int d0, int d1, int d2, int d3) {
  //  Registro de los pines
  clk_ = clk;
  cmd_ = cmd;
  d0_  = d0;
  d1_  = d1;
  d2_  = d2;
  d3_  = d3;

  //  Configuracion de resistencias pull-up
  //  Se siguio la indicacion del README de la librería SD_MMC
  pinMode(clk_, INPUT_PULLUP);
  pinMode(cmd_, INPUT_PULLUP);
  pinMode(d0_,  INPUT_PULLUP);
  pinMode(d1_,  INPUT_PULLUP);
  pinMode(d2_,  INPUT_PULLUP);
  pinMode(d3_,  INPUT_PULLUP);
}

Condicion Memoria::preparar(void) {
  //  Seteo de pines de la SD
  //  Nota: Esto queda mejor en el constructor
  //  pero se puso aquí para poder usar la
  //  sentencia return
  if(!SD_MMC.setPins(clk_, cmd_, d0_, d1_, d2_, d3_)) {
    Serial.println("Pin change failed!");
    return Condicion::Error;
  }

  //  Inicialización SD
  Serial.println("SD card initialization..."); 
  if (!SD_MMC.begin()) {          //  MMC 4-bit
    Serial.println("SD card initialization failed!");
    return Condicion::Error;
  }

  //  Inicialización LittleFS
  Serial.println("Initialize LittleFS...");                                                       
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS initialization failed!");
    return Condicion::Error;
  }

  //  Formateo de LittleFS                                                                   
  Serial.println("Formatting LittleFS...");                                                            
  LittleFS.format(); // This will erase all the files, change as needed, LittleFS is non-volatile memory
  Serial.println("LittleFS formatted successfully.");

  return Condicion::Exito;
}

Condicion Memoria::transferir(const char *filename) {

  // Open GIF file from SD card
  Serial.println("Openning GIF file from SD card...");                                     
  File sdFile = SD_MMC.open(filename);
  if (!sdFile) {
    Serial.println("Failed to open GIF file from SD card!");
    return Condicion::Error;
  }

  // Create a file in LittleFS to store the GIF
  File LittleFSFile = LittleFS.open(filename, FILE_WRITE, true);
  if (!LittleFSFile) {
    Serial.println("Failed to copy GIF in LittleFS!");
    return Condicion::Error;
  }

  // Read the GIF from SD card and write to LittleFS
  Serial.println("Copy GIF in LittleFS...");
  byte buffer[512];
  while (sdFile.available()) {
    int bytesRead = sdFile.read(buffer, sizeof(buffer));
    LittleFSFile.write(buffer, bytesRead);
  } 

  LittleFSFile.close();
  sdFile.close();

  return Condicion::Exito;
}

