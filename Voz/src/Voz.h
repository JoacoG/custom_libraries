#pragma once
#include "Arduino.h"

#include "StatusTools.h"

#include "Audio.h"

//  Volúmenes estandar por comodidad
const uint8_t VOLUMEN_BAJO  = 7;
const uint8_t VOLUMEN_MEDIO = 14;
const uint8_t VOLUMEN_ALTO  = 21;

class Voz {

  using Condicion = CondicionSimple;          //  Alias

  public:
    Voz(int bclk, int lrc, int dout);                                    //  Constructor

    Condicion preparar(void);                                            //  Inicializador

    Condicion decir(const char *audioname, int volumen = VOLUMEN_MEDIO); //  Se reproduce el audio "audioname",
                                                                         //  falla si se utiliza al estar
                                                                         //  reproduciéndose otro audio                                                                 
    void parar(void);     //  Se detiene el audio que se esté reproduciendo en el momento

    void cambiarVolumen(int volumen);

  private:
    bool ocupada_;        //  Indica si los speakers están ocupados

    //  Registro de los pines
    int dout_;
    int bclk_;
    int lrc_;       

    int vol_;             //  Debe estar dentro del rango 0...21          

    Audio audio_;         //  Audio handler

};