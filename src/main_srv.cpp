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

#include "fpga_link1_server.h"

using fpga_link1::FpgaLink1Server;
using fpga_link1::FrameType;


// Bus I2C y 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//const std::string kSerialPort("/dev/ttyS0");  // With Flow Control
const std::string kSerialPort("/dev/cu.usbserial-FTE5IS5D");   // Without Flow Control
//const std::string kSerialPort("/dev/ptyr6");   // Without Flow Control

FpgaLink1Server* com = NULL;
std::string param_port;
int param_speed;

uint32_t g_memory[1024];


void ShowUsage();
int OperationFunc(FrameType type, uint32_t address, uint32_t* data);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
      
      param_port = argv[1]; //kSerialPort; //argv[1];
      param_speed = 9600;
      
      /*
      char* endp;
      param_speed = strtoul(argv[2], &endp, 10);
      if (*endp != '\0') {
            printf("Error reading serial port speed\n");
            return 1;
      }
      */
      
      bool read;

      if (argc < 2) {
            ShowUsage();
            return 0;
      } else if (argc == 2) {
            read = true;
      } else {
            read = false;
      }

      com = new FpgaLink1Server(param_port, param_speed);
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
      printf("test_fpgalink1 [--help] [--port]] <register> [values]\n");
      printf("\n");
      printf("    <register>            Number of the FPGA register to read from or to write to\n");
      printf("    [values]              Value or values that will be written separated by space characters");
      printf("\n");
      printf("    --port=<tty-device>   Serial port connected to FPGA (default: /dev/ttyS0)\n");
      printf("    --help                Show this help\n");
      printf("\n");
      printf("EXAMPLES:\n");
      printf("\n");
      printf("    test_fpgalink1 0x1c 0x0a 0x0b\n");
      printf("    (writes in regiser 0x1c the value 0x0a and in register 0x1d the value 0x0b)\n");
      printf("\n");
      printf("    test_fpgalink1 0x1a\n");
      printf("    (read the value of regiser 0x1a)\n");
      printf("\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int OperationFunc(FrameType type, uint32_t address, uint32_t* data) {


      printf("OperationFunc: type = %d, address=0x%08X, data=0x%08X\n", (int) type, address, *data);
      
      return 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
