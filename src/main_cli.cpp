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

      bool read = false;
      int param_count = argc - optind;
      uint32_t address;
      uint32_t data;
      std::string str_address;
      std::string str_data;
      
      if (param_count == 1) {
            read = true;
            str_address = argv[optind];
      } else if (param_count == 2) {
            read = false;
            str_address = argv[optind];
            optind++;
            str_data = argv[optind];            
      } else {
            ShowUsage();
            return 1;
      }
      
            

      address = strtoul(str_address.c_str(), &endp, 0);
      if (*endp != '\0') {
            printf("Error reading address\n");
            return 1;
      }
      
      data = strtoul(str_data.c_str(), &endp, 0);
      if (*endp != '\0') {
            printf("Error reading data\n");
            return 1;
      }
      
      com = new FpgaLink1(port, speed);
      if (com->Init() != FpgaLink1::Error::No) {
            printf("Error initializing comunication driver\n");
            return 1;
      }

      FpgaLink1::Error e;
      
      if (read) {
            e = com->MemoryRD32(address, &data, 500);
            if (e != FpgaLink1::Error::No) {
                  printf("read error (%d)\n", (int) e);
                  return 1;
            }

            printf("\n");
            printf("REG[%x] -> 0x%x\n", address, data);
      } else {
            e = com->MemoryWR32(address, data, 500);
            if (e != FpgaLink1::Error::No) {
                  printf("write error (%d)\n", (int) e);
                  return 1;                  
            }

            printf("\n");
            printf("REG[%x] <- 0x%x\n", address, data);
            printf("ok\n");
      }
      
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
