// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_CODEC_H_
#define FPGA_LINK1_CODEC_H_

#include <stdint.h>

namespace fpga_link1 {

enum CommandType {
      kRead8     = 0x01,
      kRead16    = 0x02,
      kRead32    = 0x03,
      kWrite8    = 0x04,
      kWrite16   = 0x05,
      kWrite32   = 0x06,
      kInterrupt = 0x07,
      kIdle      = 0x08
};


struct Command {

      // Type of command for the microelectronic system.
      CommandType type;

      // 24-bit address, valid rage is 0x00000000 to 0x00FFFFFF.
      uint32_t address;

      // Data to write to or read from a microelectronic system register.
      union {
            uint8_t data8;
            uint16_t data16;
            uint32_t data32;
      };

            
};


struct SerializedCommand {
      uint8_t data[10];
      int size;
};


int Encoder(Command& cmd, SerializedCommand* serialized);
int Decoder(Command& cmd, SerializedCommand* serialized);


}



#endif  // FPGA_LINK1_CODEC_H_
