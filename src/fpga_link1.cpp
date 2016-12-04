// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "fpga_link1.h"
// platform
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <termios.h>
// c++
#include <cassert>
#include <cstdio>
#include <cstring>
// app
#include "codec.h"

using fpga_link1::FpgaLink1;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::FpgaLink1(std::string device, int speed_bps) : framer_(device, speed_bps) {
      
      thread_exit_ = false;
      initialized_ = false;
      tx_command_valid_ = false;
      rx_command_valid_ = false;
      func_ = nullptr;
      
      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);  
      pthread_mutex_init(&lock_, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::~FpgaLink1() {
      thread_exit_ = true;
      pthread_join(thread_, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::Init() {
      int r;

      assert(framer_.Init() == Framer::Error::No);
      
      r = pthread_create(&thread_, &thread_attr_, FpgaLink1::ThreadFn, this);
      if (r != 0) {
            return Error::ThreadCreation;
      }

      initialized_ = true;
      return Error::No;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::RegisterInterruptCallback(InterruptCallback f) {
      assert(f != nullptr);
      func_ = f;
      return Error::No;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::MemoryRD32(uint32_t address, uint32_t* data, int timeout_ms) {
      assert(initialized_ == true);

      Frame tx_cmd;
      Frame rx_cmd;
      Framer::FixedFrame tx_ser;
      Framer::FixedFrame rx_ser;
      Framer::Error e;

      struct pollfd fda[1];
      int n;

      assert((address & 0xff000000) == 0);
      
      tx_cmd.type    = FrameType::Read32;
      tx_cmd.address = address & 0x00FFFFFF;  // 24-bit address space
      tx_cmd.data32  = 0x00000000;

      //----------------------------------------------------------------------------------------------------------------          
      fda[0].fd = framer_.TxQueueFileDescriptor();
      fda[0].events = POLLOUT;
      
      n = poll(fda, 1, timeout_ms);
      if (n == 0) {
            return Error::Timeout;
      }
      
      assert((fda[0].revents & POLLOUT) == POLLOUT);

      Encoder(tx_cmd, &tx_ser);
      e = framer_.TxQueueEnqueue(tx_ser,  500);

      //----------------------------------------------------------------------------------------------------------------      

      fda[0].fd = framer_.RxQueueFileDescriptor();
      fda[0].events = POLLIN;
      
      n = poll(fda, 1, timeout_ms);
      if (n == 0) {
            return Error::Timeout;
      }
      
      assert((fda[0].revents & POLLIN) == POLLIN);
      
      e = framer_.RxQueueDequeue(&rx_ser, 500);
      Decoder(&rx_cmd, rx_ser);
      
      if (rx_cmd.type != FrameType::Read32Ack) {
            return Error::OperationNotAcknowledged;
      }

      *data = rx_cmd.data32;
      return Error::No;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::MemoryWR32(uint32_t address, uint32_t data, int timeout_ms) {
      assert(initialized_ == true);

      Frame      tx_cmd;
      Frame      rx_cmd;
      Framer::FixedFrame tx_ser;
      Framer::FixedFrame rx_ser;
      Framer::Error e;

      assert((address & 0xff000000) == 0);
            
      tx_cmd.type    = FrameType::Write32;
      tx_cmd.address = address & 0x00FFFFFF;  // 24-bit address space
      tx_cmd.data32  = data;

      printf("a=%x, d=%x\n", address, data);
      
      Encoder(tx_cmd, &tx_ser);
      e = framer_.TxQueueEnqueue(tx_ser,  500);
      //watch_.Reset();      

      
      e = framer_.RxQueueDequeue(&rx_ser, 500);
      Decoder(&rx_cmd, rx_ser);
      
      if (rx_cmd.type != FrameType::Write32Ack) {
            return Error::OperationNotAcknowledged;
      }
            
      return Error::No;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* FpgaLink1::ThreadFn(void* obj) {
      FpgaLink1* o = (FpgaLink1*) obj;
      return o->ThreadFn();
}
void* FpgaLink1::ThreadFn() {

      uint32_t idle_frame_count = 0;

      Frame tx_cmd;      
      Frame rx_cmd;
      Framer::FixedFrame tx_ser;
      Framer::FixedFrame rx_ser;
      Framer::Error e;
      
      watch_.Reset();
      watch_.Start();
      
      while (1) {

            usleep(100000);  // 100 ms

            if (thread_exit_) {
                  break;
            }
            /*
            // Transmission of kPing commands every kIdleLinkPeriod just to notify that the serial link is not broken
            if (watch_.ElapsedMilliseconds() > kIdleLinkPeriod) {
                  watch_.Reset();

                  tx_cmd.type = FrameType::Ping;
                  tx_cmd.address = 0;
                  tx_cmd.data32 = idle_frame_count;
                  
                  Encoder(tx_cmd, &tx_ser);
                  e = framer_.TxQueueEnqueue(tx_ser,  500);
                  
                  idle_frame_count++;
                  e = framer_.RxQueueDequeue(&rx_ser, 500);
                  Decoder(&rx_cmd, rx_ser);

                  // ...
            }
            */
            
            
      }  // while (1)

      return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
