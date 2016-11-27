// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_CODEC_H_
#define FPGA_LINK1_CODEC_H_

#include <stdint.h>

namespace fpga_link1 {

enum FrameType {
      // Querys (Host -> FPGA)
      kPing        = 0x01,
      kRead32      = 0x02,     
      kWrite32     = 0x03,

      // Responses (FPGA -> Host)
      kPingAck     = 0x04,
      kRead32Ack   = 0x05,
      kRead32Nack  = 0x06,
      kWrite32Ack  = 0x07,
      kWrite32Nack = 0x08,

      kInterrupt   = 0x09,
};


struct Frame {

      // Type of command for the microelectronic system (8 bits field).
      FrameType type;

      // 24-bit address, valid rage is 0x00000000 to 0x00FFFFFF (24 bits field).
      uint32_t address;

      // Data to write to or read from a microelectronic system register (32 bits field).
      
      uint32_t data32;
      


      // CRC
      uint8_t crc;      
};


struct SerializedFrame {
      uint8_t data[10];
      int size;
};


int Encoder(Frame& cmd, SerializedFrame* serialized);
int Decoder(Frame* cmd, SerializedFrame& serialized);

      class Codec {
      public:
            Codec();
            ~Codec();

            int Push(uint8_t c);            
            int Front(Frame* new_frame);
            int Pop();
            bool Found();
      private:

            uint8_t buff_[10];
            int size_;
            
            Frame frm_;
            SerializedFrame ser_;
            
            
            int count_;
            bool found_;
            
            int n_;
            
      };

}



#endif  // FPGA_LINK1_CODEC_H_
