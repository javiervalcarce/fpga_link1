// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "fpga_link1_server.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <cassert>
#include <cstdio>
#include "codec.h"

using fpga_link1::FpgaLink1Server;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1Server::FpgaLink1Server(std::string device, int speed_bps) {
      device_ = device;
      speed_ = speed_bps;
      fd_ = -1;
      thread_name_ = "fpga_link1";
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
FpgaLink1Server::~FpgaLink1Server() {
      thread_exit_ = true;
      pthread_join(thread_, NULL);
      // Terminate first the internal thread
      if (fd_ != -1) {
            close(fd_);
      }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1Server::Error FpgaLink1Server::Init() {
      int r;

      // http://stackoverflow.com/questions/26498582/
      // opening-a-serial-port-on-os-x-hangs-forever-without-o-nonblock-flag
      
      fd_ = open(device_.c_str(), O_RDWR | O_NOCTTY);
      if (fd_ == -1) {
            return Error::NoSuchDevice;
      }

      struct termios tio;     
      r = tcgetattr(fd_, &tio);
      if (r != 0) {
            return Error::Termios;
      }

      cfmakeraw(&tio);

      tio.c_cflag = CS8 | CREAD | CLOCAL;
      tio.c_cc[VMIN] = 1;
      tio.c_cc[VTIME] = 1;
      
      cfsetispeed(&tio, speed_); //B38400); //B115200);
      cfsetospeed(&tio, speed_); //B38400); //B115200);
      
      r = tcsetattr(fd_, TCSANOW, &tio);
      if (r != 0) {
            return Error::Termios;
      }

      r = pthread_create(&thread_, &thread_attr_, FpgaLink1Server::ThreadFn, this);
      if (r != 0) {
            return Error::ThreadCreation;
      }

      initialized_ = true;
      return Error::No;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* FpgaLink1Server::ThreadFn(void* obj) {
      FpgaLink1Server* o = (FpgaLink1Server*) obj;
      return o->ThreadFn();
}
void* FpgaLink1Server::ThreadFn() {

#ifdef __linux__
      pthread_setname_np(thread_, thread_name_.c_str());
#else
      // Other POSIX
      pthread_setname_np(thread_name_.c_str());
#endif
      int n;
      int i;
      int c;
      int a;
      
      uint8_t tmp[1];
      Frame rx_cmd;
      Frame tx_cmd;      
      SerializedFrame rx_ser;
      SerializedFrame tx_ser;
      uint32_t idle_frame_count = 0;
      
      watch_.Reset();
      watch_.Start();
      c = 0;

      Codec detector;
      
      while (1) {

            if (thread_exit_) {
                  break;
            }

                        
            a = 0;      
            do {
                  n = read(fd_, tmp, 1);
                  a += n;

                  detector.Push(tmp[0]);
                  
            } while (detector.Found() == false);

            // decode
            detector.Front(&rx_cmd);
            detector.Pop();

            if (rx_cmd.type != kPing) {
                  if (func_ != nullptr) {
                        func_(rx_cmd.type, rx_cmd.address, &rx_cmd.data32);
                  }
            }
            
            switch (rx_cmd.type) {
            case kPing: tx_cmd.type = kPingAck; break;
            case kRead32: tx_cmd.type = kRead32Ack; break;
            case kWrite32: tx_cmd.type = kWrite32Ack; break;
            default:
                  // Comando desconocido.
                  continue;
                  break;
            }
            tx_cmd.address = rx_cmd.address;
            tx_cmd.data32  = rx_cmd.data32;
                  
            // Frame Encoder
            Encoder(tx_cmd, &tx_ser);
            
            a = 0;
            do {
                  n = write(fd_, tx_ser.data + a, tx_ser.size - a);
                  a += n;
            } while (a < tx_ser.size);
            
            
      }  // while (1)

      return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1Server::Error FpgaLink1Server::SendInterrupt(int irq) {
      return Error::No;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Registers a callback function which will be invoked when a RD or WR frame arrives.
 */
FpgaLink1Server::Error FpgaLink1Server::RegisterCallback(OperationCallback f) {
      assert(f != nullptr);
      func_ = f;
      return Error::No;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
