#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <cassert>
#include <iostream>
#include <sys/stat.h>

struct stat results;

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(int32_t value, int32_t max)
{
    double percentage = (float)value / (float)max;
    int val  = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    const char* def_file = "./data/selfie.uart";
    const char* filename = def_file;

    if( argc > 1 )
    {
        filename = argv[1];
    }

	//
	// On charge l'image BMP en entrée du systeme
	//

    struct stat results;
    if (stat(filename, &results) != 0)
    {
        printf("(EE) An error occurs during the %s opening (stat)...\n", filename);
        exit( EXIT_FAILURE );
    }

    int fsize = results.st_size;
    char* buffer = (char*)malloc( fsize * sizeof(char) );


    FILE* f = fopen( filename, "rb" );
    if ( f == NULL )
    {
        printf("(EE) An error occurs during the %s opening (fopen)...\n", filename);
        exit( EXIT_FAILURE );
    }


    int nRead = fread( buffer, sizeof(char), fsize, f );
    if ( nRead != fsize )
    {
        printf("(EE) An error occurs during data reading (fread)...\n");
        printf("(EE) nRead = %d bytes\n", nRead);
        printf("(EE) fsize = %d bytes\n", fsize);
        exit( EXIT_FAILURE );
    }

    fclose( f );


    int uartDevice;

    printf("(II) Ouverture du port serie ttyUSB1 !\n");
    uartDevice = open("/dev/ttyUSB1", O_RDWR | O_NOCTTY );
    if(uartDevice == -1)
    {
        printf("(WW) Ouverture impossible du port ttyUSB1 !\n");
        printf("(II) Ouverture du port serie ttyUSB0 !\n");
        uartDevice = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY );
        if(uartDevice == -1)
        {
            printf("(WW) Ouverture impossible du port ttyUSB0 !\n");
            exit( EXIT_FAILURE );
        }
    }

    struct termios t;
    tcgetattr(uartDevice, &t); // recupère les attributs
    cfmakeraw(&t); // Reset les attributs
    t.c_cflag     = CREAD | CLOCAL;     // turn on READ
    t.c_cflag    |= CS8;
    t.c_cc[VMIN]  = 0;
    t.c_cc[VTIME] = 255;     // 5 sec timeout

    cfsetispeed(&t, B921600);
    cfsetospeed(&t, B921600);
    tcsetattr(uartDevice, TCSAFLUSH, &t); // envoie le tout au driver    

    int sended = 0;
    while( sended != fsize )
    {
        int paquet = ((fsize-sended) > 8192) ? 8192 : (fsize-sended);
    	int wBytes = write( uartDevice, buffer + sended, paquet);
        if( wBytes != paquet )
        {
            printf("(EE) Erreur a l'emission des données (%d != %d)\n", wBytes, 192);
            exit( EXIT_FAILURE );
        }
        sended += paquet;

        printProgress(sended, fsize);
    }

    printf("\n");

    return EXIT_SUCCESS;
}
