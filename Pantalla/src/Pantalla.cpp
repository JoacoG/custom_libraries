#include "Arduino.h"
#include "Pantalla.h"
#include "Comodidad.h"

#include <LittleFS.h>
#include <SD_MMC.h>

using Condicion = CondicionSimple;         //  Alias

// Default ctor
Pantalla::Pantalla() : tft_() {          //  Se define la pantalla
    //  Draw callback parameter
    dmaBuf_ = 0;
}

//  Con esta función se obtiene la única instancia
Pantalla& Pantalla::instancia() {   
  static Pantalla    instance; // Guaranteed to be destroyed.
                               // Instantiated on first use.
  return instance;
}

//  Inicialización de la pantalla
Condicion Pantalla::preparar(void) {
  // Al principio está desocupada
  ocupada_ = false;

  tft_.begin();
  tft_.setRotation(3);          // Adjust Rotation of your screen (0-3)
  tft_.fillScreen(TFT_BLACK);

  return Condicion::Exito;
}

//  Muestra el .gif ubicado en "filename"
Condicion Pantalla::mostrar(const char *filename) {

  if(ocupada_) {
    Serial.println("Error: pantalla ocupada");
    return Condicion::Error;
  }

  ocupada_ = true;                //  Se ocupa la pantalla

  gif_.begin(BIG_ENDIAN_PIXELS);

  if (gif_.open(filename, openStatic_, closeStatic_, readStatic_, seekStatic_, drawStatic_)) {
    tft_.startWrite();                       // The TFT chip slect is locked low
    while (gif_.playFrame(true, NULL)) {

    }
    gif_.close();
    tft_.endWrite();                         // Release TFT chip select for the SD Card Reader
  } 

  ocupada_ = false;                //  Se despcupa la pantalla

  return Condicion::Exito;
}

//  Callback functions for AnimatedGIF library
void *Pantalla::open_(const char *filename, int32_t *pFileSize) {              //  Analizar luego!
  gifFile_   = LittleFS.open(filename, FILE_READ);
  *pFileSize = gifFile_.size();

  D(if (!gifFile_) {                                                    )                                                                                                 
  D(  Serial.println("Failed to open GIF file from LittleFS!");         )         
  D(}                                                                   )      
                                                           
  return &gifFile_;
}

void Pantalla::close_(void *pHandle) {                                         //  Analizar luego!
  gifFile_.close();
}

int32_t Pantalla::read_(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {         //  Analizar luego!
  int32_t iBytesRead;
  iBytesRead = iLen;
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos;
  if (iBytesRead <= 0)
    return 0;

  gifFile_.seek(pFile->iPos);
  int32_t bytesRead = gifFile_.read(pBuf, iLen);
  pFile->iPos += iBytesRead;

  return bytesRead;
}

int32_t Pantalla::seek_(GIFFILE *pFile, int32_t iPosition) {                   //  Analizar luego!
  if (iPosition < 0)
    iPosition = 0;
  else if (iPosition >= pFile->iSize)
    iPosition = pFile->iSize - 1;

  pFile->iPos = iPosition;
  gifFile_.seek(pFile->iPos);

  return iPosition;
}

//  Draw a line of image directly on the LCD
void Pantalla::draw_(GIFDRAW *pDraw) {
  uint8_t *s;
  uint16_t *d, *usPalette;
  int x, y, iWidth, iCount;

  // Display bounds chech and cropping
  iWidth = pDraw->iWidth;
  if (iWidth + pDraw->iX > DISPLAY_WIDTH)
    iWidth = DISPLAY_WIDTH - pDraw->iX;
  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line
  if (y >= DISPLAY_HEIGHT || pDraw->iX >= DISPLAY_HEIGHT || iWidth < 1)
    return;

  // Old image disposal
  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) // restore to background color
  {
    for (x = 0; x < iWidth; x++)
    {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }

  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    pEnd = s + iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < iWidth)
    {
      c = ucTransparent - 1;
      d = &usTemp_[0][0];
      while (c != ucTransparent && s < pEnd && iCount < BUFFER_SIZE )
      {
        c = *s++;
        if (c == ucTransparent) // done, stop
        {
          s--; // back up to treat it like transparent
        }
        else // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      } // while looking for opaque pixels
      if (iCount) // any opaque pixels?
      {
        // DMA would degrtade performance here due to short line segments
        tft_.setAddrWindow(pDraw->iX + x, y, iCount, 1);
        tft_.pushPixels(usTemp_, iCount);
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
          x++;
        else
          s--;
      }
    }
  }
  else
  {
    s = pDraw->pPixels;

    // Unroll the first pass to boost DMA performance
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    if (iWidth <= BUFFER_SIZE)
      for (iCount = 0; iCount < iWidth; iCount++) usTemp_[dmaBuf_][iCount] = usPalette[*s++];
    else
      for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp_[dmaBuf_][iCount] = usPalette[*s++];

    #ifdef USE_DMA // 71.6 fps (ST7796 84.5 fps)
        tft_.dmaWait();
        tft_.setAddrWindow(pDraw->iX, y, iWidth, 1);
        tft_.pushPixelsDMA(&usTemp_[dmaBuf_][0], iCount);
        dmaBuf_ = !dmaBuf_;
    #else // 57.0 fps
        tft_.setAddrWindow(pDraw->iX, y, iWidth, 1);
        tft_.pushPixels(&usTemp_[0][0], iCount);
    #endif

        iWidth -= iCount;
        // Loop if pixel buffer smaller than width
        while (iWidth > 0)
        {
          // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
          if (iWidth <= BUFFER_SIZE)
            for (iCount = 0; iCount < iWidth; iCount++) usTemp_[dmaBuf_][iCount] = usPalette[*s++];
          else
            for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp_[dmaBuf_][iCount] = usPalette[*s++];

          #ifdef USE_DMA
                tft_.dmaWait();
                tft_.pushPixelsDMA(&usTemp_[dmaBuf_][0], iCount);
                dmaBuf_ = !dmaBuf_;
          #else
                tft_.pushPixels(&usTemp_[0][0], iCount);
          #endif

          iWidth -= iCount;
        }
  }
} /* GIFDraw() */

//  Versiones static de los callbacks
static void *openStatic_(const char *filename, int32_t *pFileSize) {
  return Pantalla::instancia().open_(filename, pFileSize);
}

static void closeStatic_(void *pHandle) {
  return Pantalla::instancia().close_(pHandle);
}

static int32_t readStatic_(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
  return Pantalla::instancia().read_(pFile, pBuf, iLen);
}

static int32_t seekStatic_(GIFFILE *pFile, int32_t iPosition) {
  return Pantalla::instancia().seek_(pFile, iPosition);
}

static void drawStatic_(GIFDRAW *pDraw) {
  return Pantalla::instancia().draw_(pDraw);
}                                   

