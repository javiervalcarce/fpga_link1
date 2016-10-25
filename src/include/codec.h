// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_CODEC_H_
#define FPGA_LINK1_CODEC_H_

#include <stdint.h>

namespace fpga_link1 {

enum CommandType {
      kNone        = 0x00,
      
      // Querys (Host -> FPGA)
      kIdle        = 0x01,
      kRead32      = 0x02,     
      kWrite32     = 0x03,

      // Responses (FPGA -> Host)
      kIdleAck     = 0x08,
      kIdleNack    = 0x09,

      kRead32Ack   = 0x0a,
      kRead32Nack  = 0x0b,

      kWrite32Ack  = 0x0c,
      kWrite32Nack = 0x0d,

      kInterrupt   = 0x0f,
};


struct Command {

      // Type of command for the microelectronic system (8 bits field).
      CommandType type;

      // 24-bit address, valid rage is 0x00000000 to 0x00FFFFFF (24 bits field).
      uint32_t address;

      // Data to write to or read from a microelectronic system register (32 bits field).
      union {
            uint8_t data8;
            uint16_t data16;
            uint32_t data32;
      };


      // CRC
      uint8_t crc;      
};


struct SerializedCommand {
      uint8_t data[10];
      int size;
};


int Encoder(Command& cmd, SerializedCommand* serialized);
int Decoder(Command* cmd, SerializedCommand& serialized);


}



#endif  // FPGA_LINK1_CODEC_H_
