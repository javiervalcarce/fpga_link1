// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_CODEC_H_
#define FPGA_LINK1_CODEC_H_

#include <stdint.h>
#include "framer.h"


namespace fpga_link1 {

      enum class FrameType {
            // Querys (Host -> FPGA)
                  Ping        = 0x01,
                  Read32      = 0x02,     
                  Write32     = 0x03,

            // Responses (FPGA -> Host)
                  PingAck     = 0x04,
                  Read32Ack   = 0x05,
                  Read32Nack  = 0x06,
                  Write32Ack  = 0x07,
                  Write32Nack = 0x08,

                  Interrupt   = 0x09,
                  };

      
      // The Frame
      struct Frame {

            // Type of command for the microelectronic system (6 bits field).
            FrameType type;

            // 24-bit address, valid rage is 0x00000000 to 0x00FFFFFF (24 bits field).
            uint32_t address;

            // Data to write to or read from a microelectronic system register (32 bits field).
            uint32_t data32;
      };


      int Encoder(Frame& cmd, fpga_link1::Framer::FixedFrame* s);
      int Decoder(Frame* cmd, fpga_link1::Framer::FixedFrame& s);


}



#endif  // FPGA_LINK1_CODEC_H_
