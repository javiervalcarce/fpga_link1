// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "codec.h"

#include <cassert>
#include <cstdio>

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
            tmp[0] = cmd.type;
            tmp[1] = (cmd.address & 0x00ff0000) >> 16;
            tmp[2] = (cmd.address & 0x0000ff00) >> 8;
            tmp[3] = (cmd.address & 0x000000ff) >> 0;
            tmp[4] = cmd.data8;
            tmp[5] = 0x00;
            tmp[6] = 0x00;
            tmp[7] = 0x00;
            tmp[8] = 0xFF; // CRC
            break;
          
      case kWrite32:
            tmp[0] = cmd.type;
            tmp[1] = (cmd.address & 0x00ff0000) >> 16;
            tmp[2] = (cmd.address & 0x0000ff00) >> 8;
            tmp[3] = (cmd.address & 0x000000ff) >> 0;
            tmp[4] = (cmd.data32 & 0xff000000) >> 24;
            tmp[5] = (cmd.data32 & 0x00ff0000) >> 16;
            tmp[6] = (cmd.data32 & 0x0000ff00) >> 8;
            tmp[7] = (cmd.data32 & 0x000000ff) >> 0;
            tmp[8] = 0xFF; // CRC
            break;

      case kIdle:
            tmp[0] = cmd.type;
            tmp[1] = 0xa5;
            tmp[2] = 0xa5;
            tmp[3] = 0xa5;
            tmp[4] = 0xa5;
            tmp[5] = 0xa5;
            tmp[6] = 0xa5;
            tmp[7] = 0xa5;
            tmp[8] = 0xFF; // CRC
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

      printf("Encoder: Encoded Frame    : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
             serialized->data[0],
             serialized->data[1],
             serialized->data[2],
             serialized->data[3],
             serialized->data[4],
             serialized->data[5],
             serialized->data[6],
             serialized->data[7],
             serialized->data[8],
             serialized->data[9]);

      
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int fpga_link1::Decoder(Command* cmd, SerializedCommand& serialized) {

      uint8_t tmp[9];

      assert(serialized.size == 10);

      printf("Decoder: Encoded Frame    : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
             serialized.data[0],
             serialized.data[1],
             serialized.data[2],
             serialized.data[3],
             serialized.data[4],
             serialized.data[5],
             serialized.data[6],
             serialized.data[7],
             serialized.data[8],
             serialized.data[9]);

            
      tmp[8] = ((serialized.data[8] & 0b00000001) << 7) | ((serialized.data[9] & 0b01111111) >> 0);
      tmp[7] = ((serialized.data[7] & 0b00000011) << 6) | ((serialized.data[8] & 0b01111110) >> 1);
      tmp[6] = ((serialized.data[6] & 0b00000111) << 5) | ((serialized.data[7] & 0b01111100) >> 2);
      tmp[5] = ((serialized.data[5] & 0b00001111) << 4) | ((serialized.data[6] & 0b01111000) >> 3);
      tmp[4] = ((serialized.data[4] & 0b00011111) << 3) | ((serialized.data[5] & 0b01110000) >> 4);
      tmp[3] = ((serialized.data[3] & 0b00111111) << 2) | ((serialized.data[4] & 0b01100000) >> 5);      
      tmp[2] = ((serialized.data[2] & 0b01111111) << 1) | ((serialized.data[3] & 0b01000000) >> 6);
      tmp[1] = ((serialized.data[0] & 0b00000001) << 7) | ((serialized.data[1] & 0b01111111) >> 0);
      tmp[0] = ((serialized.data[0] & 0b00111110) >> 1);

      

      switch (tmp[0]) {
      case kWrite32:
            cmd->type    = static_cast<CommandType>(tmp[0]);
            cmd->address = tmp[1] << 16 | tmp[2] <<  8 | tmp[3] << 0;
            cmd->data32  = tmp[4] << 24 | tmp[5] << 16 | tmp[6] << 8 | tmp[7] << 0;
            // CRC TODO
            break;
      case kIdle:
            cmd->type    = static_cast<CommandType>(tmp[0]);
            cmd->address = 0x00a5a5a5;
            cmd->data32  = 0xa5a5a5a5;
            break;
      default:
            assert(false);
      }

      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
