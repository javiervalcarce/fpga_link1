// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
/**
 * Programa de manipulación de bits y registros de una FPGA con el diseÃ±o FPGALINK1.
 * Ver el directorio vhdl/
 *
 *
 *
 */
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "fpga_link1.h"

using fpga_link1::FpgaLink1;


// Bus I2C y 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1* com = NULL;


//const std::string kSerialPort("/dev/ttyS0");  // With Flow Control
const std::string kSerialPort("/dev/cu.usbserial-FTE5IS5D");   // Without Flow Control

void ShowUsage();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
      
      std::string port = kSerialPort;
      int speed = 9600;
      bool background = false;
      int address;
      int data;

      // Process comand line options
      static struct option long_options[] = {
            { "background",    no_argument,       0, 'b' },
            { "port",          required_argument, 0, 'p' },
            { "speed",         required_argument, 0, 's' },
            { 0, 0, 0, 0 }
      };

      char* endp;
      int option_index = 0;
      int c;
      
      if (argc == 1) {
            printf("%s [--background] [--port] [--speed] [ADDRESS] [DATA]\n", argv[0]);
            exit(0);
      }

      while (1) {

            c = getopt_long (argc, argv, "bp:s:", long_options, &option_index);
            if (c == -1) {
                  break;
            }
	    
            switch (c) {
            case 0:
                  // If this option set a flag, do nothing else now
                  if (long_options[option_index].flag != 0) {
                        break;
                  }

                  printf ("option %s", long_options[option_index].name);
                  if (optarg) {
                        printf (" with arg %s", optarg);
                  }

                  printf ("\n");
                  break;
                  
            case 'p':
                  port = optarg;
                  break;

            case 's':
                  speed = strtoul(optarg, &endp, 0);
                  if (*endp != '\0') {
                        printf("Error reading serial port speed\n");
                        return 1;
                  }
                  break;

            case 'b':
                  background = true;
                  break;

            default:
                  return 1;
            }
      }

      /* Print any remaining command line arguments (not options). */
      if (optind < argc) {
            printf ("non-option ARGV-elements: ");
            while (optind < argc) {
                  printf ("%s ", argv[optind++]);
            }
            putchar ('\n');
      }

      
      bool read = false;

      com = new FpgaLink1(port, speed);
      if (com->Init() != FpgaLink1::Error::No) {
            printf("Error initializing comunication driver\n");
            return 1;
      }
      /*
      FpgaLink1::Error e;
      int reg;
      int val;
      uint32_t ui32;
      
      if (read) {
            reg = atoi(argv[1]);

            printf("Reading from %d\n", reg);
            e = com->MemoryRD32(reg, &ui32);
            if (e != FpgaLink1::Error::No) {
                  printf("error\n");
                  return 1;
            }

            printf("0x%02x\n", ui32);
      } else {

            reg = atoi(argv[1]);
            val = atoi(argv[2]);
            ui32 = static_cast<uint32_t>(val);


            printf("Writing %d at %d\n", val, reg);
            
            e = com->MemoryWR32(reg,  ui32);
            if (e != FpgaLink1::Error::No) {
                  printf("error\n");
                  return 1;                  
            }
            printf("ok\n");
      }
      */

      
      if (background) {
            while (1) {
                  sleep(1);
            }
      }
      
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowUsage() {
      printf("\n");
      printf("Usage:\n");
      printf("fpgalink1_cli [--port] [--speed] ADDRESS DATA\n");
      printf("\n");
      printf("\n");
      printf("EXAMPLES:\n");
      printf("\n");
      printf("    fpgalink1_cli 0x000234 0x00aabbf1\n");
      printf("    (writes in the 32-bit register at address 0x000234 the value 0x00aabbf1)\n");
      printf("\n");
      printf("    fpgalink1_cli 0x1a\n");
      printf("    (read the value of regiser 0x1a)\n");
      printf("\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
