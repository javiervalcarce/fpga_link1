// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
/**
 * Programa de manipulación de bits y registros de una FPGA con el diseño FPGALINK1.
 * Ver el directorio vhdl/
 *
 */

#include <stdint.h>
#include <unistd.h>
#include <getopt.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "fpga_link1.h"

using fpga_link1::FpgaLink1;

// Bus I2C y 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1* driver = NULL;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {


      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void printhelp(const char* cmd, char* i2cbus_fname, uint8_t iofpga_addr) {
      printf("\n"
             "Usage: fpgalink1 <register> [value]\n"
             "\n"
             "<register>  Number of the FPGA register to read from or to write to. In decimal or hexadecimal (0x)\n"
             "[value]     Value that will be written, in decimal o hexadecimal (0x) format"
             "\n"
             "Actions: (executed in the order they are written in command line)\n"
             " --bus=<i2c-device>         Serial Port (default: /dev/ttyS0)\n"
             " --help                     Show this help\n"
             "\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
