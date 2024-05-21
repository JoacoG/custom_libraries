#pragma once
#include "Arduino.h"
#include "StatusTools.h"

class Memoria {
  using Condicion = CondicionSimple;          //  Alias

  public:
    Memoria(int clk, int cmd, int d0, int d1, int d2, int d3);  //  Constructor
    Condicion preparar(void);                                   //  Inicializar la memoria
    Condicion transferir(const char *filename);                 //  Transferir el archivo "filename"
                                                                //  desde la SD a la flash
  private:
    //  Registro de los pines
    int clk_;
    int cmd_;
    int d0_;
    int d1_;
    int d2_;
    int d3_;

};

