#include "StdAfx.h"

extern void windows_with_traslape(int);
extern void convertidor();
extern void desconvertidor();
extern void borrar_archivos();

int contador = 0;

int main()
{
    contador = 0;
    convertidor();
    windows_with_traslape(0);
    desconvertidor();
    borrar_archivos();
    return 0;
}
