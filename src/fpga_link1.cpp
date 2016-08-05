// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "fpga_link1.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <cassert>
#include <cstdio>
#include "codec.h"

using fpga_link1::FpgaLink1;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::FpgaLink1(std::string device) {

      device_ = device;
      fd_ = -1;
      thread_name_ = "fpga_link1";
      thread_exit_ = false;
      initialized_ = false;
      
      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);  
      pthread_mutex_init(&lock_, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::~FpgaLink1() {

      // Terminate first the internal thread
      if (fd_ != -1) {
            close(fd_);
      }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::Init() {
      int r;

      // http://stackoverflow.com/questions/26498582/opening-a-serial-port-on-os-x-hangs-forever-without-o-nonblock-flag
      
      fd_ = open(device_.c_str(), O_RDWR);
      if (fd_ == -1) {
            return kErrorNoSuchDevice;
      }

      struct termios tio;
      
      r = tcgetattr(fd_, &tio);
      if (r != 0) {
            return kErrorTermios;
      }

      cfmakeraw(&tio);

      tio.c_cflag = CS8 | CREAD | CLOCAL;
      tio.c_cc[VMIN] = 1;
      tio.c_cc[VTIME] = 1;
      
      cfsetispeed(&tio, B38400); //B115200);
      cfsetospeed(&tio, B38400); //B115200);
      
      r = tcsetattr(fd_, TCSANOW, &tio);
      if (r != 0) {
            return kErrorTermios;
      }

      r = pthread_create(&thread_, &thread_attr_, FpgaLink1::ThreadFn, this);
      if (r != 0) {
            return kErrorThreadCreation;
      }

      initialized_ = true;
      return kErrorNo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::MemoryRD(int reg, uint8_t* data) {
      assert(initialized_ == true);
      *data = 0xfe;
      return kErrorNo;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::MemoryRD(int reg, uint8_t* data, int len) {
      assert(initialized_ == true);
      int i;

      for (i = 0; i < len; i++) {
            data[i] = 0xfe;
      }

      return kErrorNo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::MemoryWR(int reg, uint8_t  data) {      
      assert(initialized_ == true);

      Command cmd;
      SerializedCommand octet_stream;
      
      cmd.type = kWrite8;
      cmd.address = static_cast<uint32_t>(reg) & 0x00FFFFFF;  // 24-bit address space
      cmd.data8 = data;

      Encoder(cmd, &octet_stream);
      
      int n;


      
      n = 0;
      do {
            n += write(fd_, octet_stream.data, octet_stream.size);
            if (n == -1) {
                  return kErrorIO;
            }
      } while (n < octet_stream.size);
      
      return kErrorNo;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::MemoryWR32(int reg, uint32_t data) {
      assert(initialized_ == true);

      Command cmd;
      SerializedCommand octet_stream;
      
      cmd.type = kWrite32;
      cmd.address = static_cast<uint32_t>(reg) & 0x00FFFFFF;  // 24-bit address space
      cmd.data32 = data;

      Encoder(cmd, &octet_stream);
      
      int n;

      printf("Sending this: ");
      for (int i = 0; i < octet_stream.size; i++) {
            printf("0x%02x ", octet_stream.data[i]);
      }
      printf("\n");

      
      n = 0;
      do {
            n += write(fd_, octet_stream.data, octet_stream.size);
            if (n == -1) {
                  return kErrorIO;
            }
      } while (n < octet_stream.size);
      
      return kErrorNo;
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::MemoryWR(int reg, uint8_t* data, int len) {
      assert(initialized_ == true);
      return kErrorNo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::FifoRD(int reg, uint8_t* data, int len) {
      assert(initialized_ == true);
      return kErrorNo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::FifoWR(int reg, uint8_t* data, int len) {
      assert(initialized_ == true);
      return kErrorNo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* FpgaLink1::ThreadFn(void* obj) {
      FpgaLink1* o = (FpgaLink1*) obj;
      return o->ThreadFn();
}
void* FpgaLink1::ThreadFn() {

      // Cambio el nombre de este hilo ejecutor de tareas por el de la nueva tarea que voy a ejecutar, esto
      // es EXTREMADAMENTE útil cuando depuramos el proceso con gdb (comando info threads)


#ifdef __linux__
      pthread_setname_np(thread_, thread_name_.c_str());
#else
      // Other POSIX
      pthread_setname_np(thread_name_.c_str());
#endif

      //int wait_for;
      //int r;

      while (1) {

            if (thread_exit_) {
                  break;
            }
            usleep(1e6);
      }

      return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
