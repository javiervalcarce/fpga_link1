// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "codec.h"

#include <cassert>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  bit 69              64 63               40 39                              8 7                            0
  +------------------------------------------------------------------------------------------------------+
  | OPCODE (6 bits) | ADDRESS (24 bits) | DATA (32 bits)                  | CRC (8 bits)                 |
  +------------------------------------------------------------------------------------------------------+
*/
int fpga_link1::Encoder(Command& cmd, SerializedCommand* serialized) {

      uint8_t tmp[9];
      
      switch (cmd.type) {
      case kWrite8:
            tmp[0] = cmd.type << 4;
            tmp[1] = (cmd.address & 0x00ff0000) >> 16;
            tmp[2] = (cmd.address & 0x0000ff00) >> 8;
            tmp[3] = (cmd.address & 0x000000ff) >> 0;
            tmp[4] = cmd.data8;
            tmp[5] = 0x00;
            tmp[6] = 0x00;
            tmp[7] = 0x00;
            tmp[5] = 0xFF; // CRC
            break;
          
      case kWrite32:
            tmp[0] = cmd.type << 4;
            tmp[1] = (cmd.address & 0x00ff0000) >> 16;
            tmp[2] = (cmd.address & 0x0000ff00) >> 8;
            tmp[3] = (cmd.address & 0x000000ff) >> 0;
            tmp[4] = (cmd.data32 & 0xff000000) >> 24;
            tmp[5] = (cmd.data32 & 0x00ff0000) >> 16;
            tmp[6] = (cmd.data32 & 0x0000ff00) >> 8;
            tmp[7] = (cmd.data32 & 0x000000ff) >> 0;
            tmp[5] = 0xFF; // CRC
            break;
      default:
            assert(false);
      }

      serialized->size = 10;
      
      // First transmitted byte over serial link is serialized->data[0]
      
      serialized->data[9] = ((tmp[8] & 0b01111111) << 0) | ((tmp[0] & 0b00000000) >> 0);  // 7+0
      serialized->data[8] = ((tmp[7] & 0b00111111) << 1) | ((tmp[8] & 0b10000000) >> 7);  // 6+1
      serialized->data[7] = ((tmp[6] & 0b00011111) << 2) | ((tmp[7] & 0b11000000) >> 6);  // 5+2
      serialized->data[6] = ((tmp[5] & 0b00001111) << 3) | ((tmp[6] & 0b11100000) >> 5);  // 4+3
      serialized->data[5] = ((tmp[4] & 0b00000111) << 4) | ((tmp[5] & 0b11110000) >> 4);  // 3+4
      serialized->data[4] = ((tmp[3] & 0b00000011) << 5) | ((tmp[4] & 0b11111000) >> 3);  // 2+5
      serialized->data[3] = ((tmp[2] & 0b00000001) << 6) | ((tmp[3] & 0b11111100) >> 2);  // 1+6
      serialized->data[2] = ((tmp[1] & 0b00000000) << 7) | ((tmp[2] & 0b11111110) >> 1);  // 0+7
      serialized->data[1] = ((tmp[1] & 0b01111111) << 0) | ((tmp[2] & 0b00000000) >> 0);  // 7+0
      serialized->data[0] = ((tmp[0] & 0b00111111) << 1) | ((tmp[1] & 0b10000000) >> 7);  // 6+1

      // First byte of encoded frame has its MSB set to '1', all others set to '0'.
      serialized->data[0] |= 0b10000000;
      
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int fpga_link1::Decoder(Command& cmd, SerializedCommand* serialized) {
      
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
