//  Librería cuya función es usar una pantalla ILI9341 3.2"  
//  SPI TFT LCD para reproducir gif desde una partición de la   
//  memoria flash con sistema de archivos LittleFS en un ESP32-S3
//
//  No se usará el táctil

#pragma once
#include "Arduino.h"

#include <FS.h>

#include <SPI.h>
#include <TFT_eSPI.h>

#include <AnimatedGIF.h>

#include "StatusTools.h"

//  ************************* GIFDraw  *************************
#define DISPLAY_WIDTH  tft_.width()                // La anchura de nuestra pantalla
#define DISPLAY_HEIGHT tft_.height()               // La altura de nuestra pantalla
#define BUFFER_SIZE    320 //256                   // Optimum is >= GIF width or integral division of width

class Pantalla {

  using Condicion = CondicionSimple;   //  Alias

  public:

    // Se obtiene la única instancia de la clase 
    static Pantalla& instancia();

    // Se inicializa la pantalla
    Condicion preparar(void);

    // Se muestra el gif "filename" que 
    // debe estar guardado en la memoria 
    // flash con partición LittleFS
    Condicion mostrar(const char *fileName);

    //  Callbacks para los gifs (Librería AnimatedGIF)
    //  NO UTILIZAR !!!
    void *  open_ (const char *filename, int32_t *pFileSize);
    void    close_(void *pHandle);
    int32_t read_ (GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
    int32_t seek_ (GIFFILE *pFile, int32_t iPosition);
    void    draw_ (GIFDRAW *pDraw);

  private:

    Pantalla();                    // Constructor

    //  Indica si la pantalla está ocupada
    bool ocupada_;

    AnimatedGIF gif_;
    File gifFile_;                          // File object for the GIF file
    TFT_eSPI tft_;                          // Objeto de la pantalla

    //  Draw callback parameters
    #ifdef USE_DMA
      uint16_t usTemp_[2][BUFFER_SIZE];     // Global to support DMA use
    #else
      uint16_t usTemp_[1][BUFFER_SIZE];     // Global to support DMA use
    #endif

    bool dmaBuf_;

  public:

      Pantalla(Pantalla const&)        = delete;
      void operator=(Pantalla const&)  = delete;

      // Note: Scott Meyers mentions in his Effective Modern
      //       C++ book, that deleted functions should generally
      //       be public as it results in better error messages
      //       due to the compilers behavior to check accessibility
      //       before deleted status

};

//  Versiones static de los callbacks
//  NO UTILIZAR !!!
static void *  openStatic_  (const char *filename, int32_t *pFileSize);
static void    closeStatic_ (void *pHandle);
static int32_t readStatic_  (GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
static int32_t seekStatic_  (GIFFILE *pFile, int32_t iPosition);
static void    drawStatic_  (GIFDRAW *pDraw);  