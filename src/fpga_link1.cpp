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
FpgaLink1::Error FpgaLink1::MemoryRD08(int reg, uint8_t* data) {
      assert(initialized_ == true);
      *data = 0xfe;
      return kErrorNo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::MemoryRD32(int reg, uint32_t* data) {
      assert(initialized_ == true);
      *data = 0xfe;
      return kErrorNo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
FpgaLink1::Error FpgaLink1::MemoryRD(int reg, uint8_t* data, int len) {
      assert(initialized_ == true);
      int i;

      for (i = 0; i < len; i++) {
            data[i] = 0xfe;
      }

      return kErrorNo;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::MemoryWR08(int reg, uint8_t  data) {      
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

      n = RobustWR(fd_, octet_stream.data, octet_stream.size, 1000);
      if (n != octet_stream.size) {
            return kErrorIO;
      }

      // Receive
      n = RobustRD(fd_, octet_stream.data, octet_stream.size, 1000);
      if (n != octet_stream.size) {
            return kErrorIO;
      }

      Decoder(cmd, &octet_stream);


      if (cmd.type != kWrite32Nack) {
            return kErrorProtocol;
      }

      if (cmd.type != kWrite32Ack) {
            return kErrorProtocol;
      }
      
      return kErrorNo;
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
FpgaLink1::Error FpgaLink1::MemoryWR(int reg, uint8_t* data, int len) {
      assert(initialized_ == true);
      return kErrorNo;
}
*/

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

      uint8_t buffer[64];
      int n;
            
      
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
      Command cmd;
      SerializedCommand octet_stream;
      
      cmd.type = kIdle;
      cmd.address = 0x00FFFFFF;  // 24-bit address space
      cmd.data32 = 0xAABBCCDD;

      Encoder(cmd, &octet_stream);

      watch_.Reset();
      watch_.Start();
      
      while (1) {

            if (thread_exit_) {
                  break;
            }

            if (watch_.ElapsedMilliseconds() > 200) {
            
                  n = RobustWR(fd_, buffer, 10, 200);
                  if (n == 0) {
                        // Timeout
                  }

                  watch_.Reset();
            }
            

            //poll

            n = RobustRD(fd_, buffer, 10, 50);
            if (n == 0) {
                  // Timeout
            } else {
                  printf("We have received something (%d characters)!\n", n);
            }
            
            
      }

      return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read() y write() son funciones de muy bajo nivel...
//
// Funciones de lectura y escritura de alto nivel, con reintento hasta completar los |n| caracteres y 
// temporización de 100 ms por si acaso. Devuelven |n| si todo fue bien, o un número inferior si algo
// falló
//
int FpgaLink1::RobustWR(int fd, uint8_t* s, int n, int timeout_ms) {

      int sr;
      int wr;
      int total;
      struct timeval timeout;
      fd_set set;

      FD_ZERO(&set);          // clear the set 
      FD_SET(fd, &set);       // add our file descriptor to the set

      timeout.tv_sec  = (timeout_ms / 1000); 
      timeout.tv_usec = (timeout_ms % 1000) * 1000;   

      total = 0;
      while (total < n) {
            
            sr = select(fd + 1, NULL, &set, NULL, &timeout);            
            
            /**/ if (sr == -1) { 
                  // Error E/S
                  return total; 
            } else if (sr ==  0) { 
                  // Timeout!
                  return total; 
            }

            wr = (int) write(fd, (const void*) (s + total), n - total);
            if (wr == -1) {
                  return total;
            }

            total += wr;
      }

      return total;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int FpgaLink1::RobustRD(int fd, uint8_t* s, int n, int timeout_ms) {

      int sr;
      int rd;
      int total;
      struct timeval timeout;
      fd_set set;

      FD_ZERO(&set);          // clear the set 
      FD_SET(fd, &set);       // add our file descriptor to the set

      timeout.tv_sec  = (timeout_ms / 1000); 
      timeout.tv_usec = (timeout_ms % 1000) * 1000;   

      total = 0;
      while (total < n) {

            sr = select(fd + 1, &set, NULL, NULL, &timeout);            
            /**/ if (sr == -1) { 
                  //printf("rd sel-1\n");
                  // select error 
                  return total; 
            } else if (sr ==  0) { 
                  //printf("rd sel 0\n");
                  // timeout
                  return total; 
            }  

            rd = (int) read(fd, (void*) (s + total), n - total);
            if (rd == -1) {
                  return total;
            }

            total += rd;
      }

      return total;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
