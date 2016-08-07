// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-


#include "codec.h"
#include <cstdio>
#include <cstring>
#include <cassert>



int main(int argc, char** argv) {

      fpga_link1::Command cmd1;
      fpga_link1::Command cmd2;
      fpga_link1::SerializedCommand ser;
      //fpga_link1::SerializedCommand ser2;
      
      cmd1.type = fpga_link1::kWrite32;
      cmd1.address = 0x00253545;
      cmd1.data32 = 0xA5B5C5D5;

      fpga_link1::Encoder(cmd1, &ser);
/*
      printf("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", 
             ser.data[0],
             ser.data[1],
             ser.data[2],
             ser.data[3],
             ser.data[4],
             ser.data[5],
             ser.data[6],
             ser.data[7],
             ser.data[8],
             ser.data[9]);
*/
      
      assert((ser.data[0] & 0x80) == 0x80);
      assert((ser.data[1] & 0x80) == 0x00);
      assert((ser.data[2] & 0x80) == 0x00);
      assert((ser.data[3] & 0x80) == 0x00);
      assert((ser.data[4] & 0x80) == 0x00);
      assert((ser.data[5] & 0x80) == 0x00);
      assert((ser.data[6] & 0x80) == 0x00);
      assert((ser.data[7] & 0x80) == 0x00);
      assert((ser.data[8] & 0x80) == 0x00);
      assert((ser.data[9] & 0x80) == 0x00);


      fpga_link1::Decoder(&cmd2, ser);

      assert(memcmp((char*) &cmd1, (char*) &cmd2, sizeof(fpga_link1::Command)) == 0);
      //assert(cmd1 == cmd2);

      return 0;
}
