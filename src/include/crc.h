// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_CRC_H_
#define FPGA_LINK1_CRC_H_

#include "codec.h"

namespace fpga_link1 {

      // Returns 0 if CRC is ok, 1 if not match (crc error)
      int CheckFrameCRC(Frame& cmd);

      // Set the CRC corresponding to the frame contents. Returns always 0.
      int WriteFrameCRC(Frame* cmd);


}


#endif  // FPGA_LINK1_CRC_H_
