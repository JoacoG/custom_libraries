#include "Arduino.h"
#include "Voz.h"

#include "SD_MMC.h"
#include "FS.h"

using Condicion = CondicionSimple;          //  Alias

Voz::Voz(int bclk, int lrc, int dout) {                 //  Constructor
  //  Inicialización del bus de audio
  if(audio_.setPinout(bclk, lrc, dout) == ESP_OK) {                    
    //  Registro interno de los pines
    dout_ = dout;
    bclk_ = bclk;
    lrc_ = lrc;    
  } else {          //  Si da error
    Serial.println("Error: no se pudieron establecer los pines de la Voz");
  }
}

Condicion Voz::preparar(void) {                       // Inicializador
  ocupada_ = false;                                   // Al principio no está ocupada

  audio_.setFileLoop(false);                          // Que no se repitan los audios
  audio_.setBalance(0);                               // Balancea los canales izquierdo y derecho

  audio_.setVolume(VOLUMEN_MEDIO);                    // Volumen por defecto
  vol_ = VOLUMEN_MEDIO;

  return Condicion::Exito;
}

Condicion Voz::decir(const char *audioname, int volumen) {        // Se reproduce el audio "audioname",
                                                                  // falla si se utiliza al estar
                                                                  // reproduciéndose otro audio
  if(ocupada_) {
    Serial.println("Error: voz ocupada");
    return Condicion::Error;
  }
  
  ocupada_ = true;          //  Ahora está ocupada porque se va a decir algo

  audio_.setVolume(volumen);
  vol_ = volumen;

  audio_.connecttoFS(SD_MMC, audioname);
  Serial.print("Se conectó con ");
  Serial.println(audioname);

  audio_.loop();                    //  Esta acción puede ser necesaria para "activar" la flag de is_running
                                    //  Si no es necesaria se borra
  while(audio_.isRunning()) {
    audio_.loop();
  }

  ocupada_ = false;         //  Ya se mostró todo

  return Condicion::Exito;
}

void Voz::parar(void) {                             //  Se detiene el audio que se esté reproduciendo en el momento
  audio_.stopSong();
  ocupada_ = false;                                 //  Ya no está ocupada
}

void Voz::cambiarVolumen(int volumen) {
  audio_.setVolume(volumen);
  vol_ = volumen;  
}