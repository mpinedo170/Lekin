#include "StdAfx.h"

int min2(int a, int b)
{
    if (a <= b)
        return (a);
    else
        return (b);
}

int min2(int a, int b, int c)
{
    if (a <= b && a <= c)
        return (a);
    else if (b <= a && b <= c)
        return (b);
    else
        return (c);
}

int min2(int a, int b, int c, int d)
{
    if (a <= b && a <= c && a <= d)
        return (a);
    else if (b <= a && b <= c && b <= d)
        return (b);
    else if (c <= a && c <= b && c <= d)
        return (c);
    else
        return (d);
}

int max2(int a, int b)
{
    if (a < b)
        return (b);
    else
        return (a);
}

int pos(int a)
{
    if (a > 0)
        return (a);
    else
        return (0);
}

double pos(double a)
{
    if (a > 0)
        return (a);
    else
        return (0);
}

FILE* open_file(char* input_file_name, char* access_mode)
{
    FILE* fp;
    char* file_name;

    // Lineas arregladas por RENE

    // UNIX version:
    char* home_dir = getenv("HOME");
    file_name = (char*)malloc(_tcslen(home_dir) + _tcslen("/DEC/") + _tcslen(input_file_name) + 1);
    //        sprintf( file_name , "%s%s%s" , home_dir , "/DEC/" , input_file_name ) ;
    sprintf(file_name, "%s", input_file_name);

    // PC version:
    // Augusto 14.8.96
    //      file_name = (char *) malloc ( _tcslen("C:\\users\\singer\\DEC\\") + _tcslen(input_file_name) + 1 )
    //      ; sprintf( file_name , "%s%s" , "C:\\users\\singer\\DEC\\" , input_file_name ) ;

    //                file_name = (char *) malloc ( _tcslen("c:\\users\\anunez\\DEC\\") +
    //                _tcslen(input_file_name) + 1 ) ; file_name = (char *) malloc ( _tcslen(input_file_name)
    //                + 1 ) ; sprintf( file_name , "%s" , input_file_name ) ;

    // Fin Lineas Arregladas por RENE

    // Augusto 19.8.96
    //              file_name = (char *) malloc ( _tcslen(input_file_name) + 1 ) ;
    //              sprintf( file_name , "%s" , input_file_name ) ;
    //         printf(" \n Tamaño del stack : %u ", stackavail());
    //    printf(" \n Abriendo archivo : %s", input_file_name);

    if ((fp = fopen(file_name, access_mode)) == NULL)
        printf("ERROR opening file %s with access mode %s\n", file_name, access_mode);
    delete[] file_name;
    return (fp);
}
