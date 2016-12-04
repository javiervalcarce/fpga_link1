// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
/**
 * Programa de manipulación de bits y registros de una FPGA con el diseÃ±o FPGALINK1.
 * Ver el directorio vhdl/
 *
 */
// platform
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
// c++
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
// 
#include "fpga_link1_server.h"

using fpga_link1::FpgaLink1Server;
using fpga_link1::FrameType;



// Bus I2C 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//const std::string kSerialPort("/dev/ttyS0");  // With Flow Control
const std::string kSerialPort("/dev/cu.usbserial-FTE5IS5D");   // Without Flow Control
//const std::string kSerialPort("/dev/ptyr6");   // Without Flow Control


FpgaLink1Server* com = NULL;
std::string port;
int speed;
uint32_t g_memory[1024];

void ShowUsage();
int OperationFunc(FrameType type, uint32_t address, uint32_t* data);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
      
      port = kSerialPort;
      speed = 9600;

      // Process comand line options
      static struct option long_options[] = {
            { "port",          required_argument, 0, 'p' },
            { "speed",         required_argument, 0, 's' },
            { 0, 0, 0, 0 }
      };

      char* endp;
      int option_index = 0;
      int c;
      
      if (argc == 1) {
            printf("%s [--port] [--speed]\n", argv[0]);
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






      
      com = new FpgaLink1Server(port, speed);
      if (com->Init() != FpgaLink1Server::Error::No) {
            printf("Error initializing comunication driver\n");
            return 1;
      }

      while (1) {
            g_memory[0] = 0xffffffff;
            sleep(5);
            g_memory[0] = 0xa5a5a5a5;
            sleep(5);
      }
      
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowUsage() {
      printf("\n");
      printf("Usage:\n");
      printf("fpgalink1_srv [--port] [--speed]\n");
      printf("\n");
      printf("    --port=<tty-device>   Serial port connected to FPGA (default: /dev/ttyS0)\n");
      printf("    --port=<speed>        Serial port speed in bps (default: 9600)\n");
      printf("    --help                Show this help\n");
      printf("\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int OperationFunc(FrameType type, uint32_t address, uint32_t* data) {
      printf("OperationFunc: type = %d, address=0x%08X, data=0x%08X\n", (int) type, address, *data);
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
