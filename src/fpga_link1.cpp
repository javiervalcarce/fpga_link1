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
      tx_command_valid_ = false;
      rx_command_valid_ = false;
      func_ = nullptr;
      
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
FpgaLink1::Error FpgaLink1::RegisterInterruptCallback(InterruptCallback f) {
      assert(f != nullptr);
      func_ = f;
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
      
      cmd.type = kWrite32;
      cmd.address = static_cast<uint32_t>(reg) & 0x00FFFFFF;  // 24-bit address space
      cmd.data32 = data;

      // Empty rx buffer if not empty
      pthread_mutex_lock(&lock_);
      if (rx_command_valid_) {
            rx_command_valid_ = false;
      } 
      pthread_mutex_unlock(&lock_);

      // Leave the tx command in the tx buffer
      pthread_mutex_lock(&lock_);
      tx_command_ = cmd;
      tx_command_valid_ = true; 
      pthread_mutex_unlock(&lock_);
            
      
      // Wait for the reception of the answer in the rx buffer
      while (1) {
            pthread_mutex_lock(&lock_);
            if (rx_command_valid_) {
                  cmd = rx_command_;
                  rx_command_valid_ = false;
                  pthread_mutex_unlock(&lock_);
                  break;
            } 
            pthread_mutex_unlock(&lock_);

            usleep(kWaitForValidSleep);
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

#ifdef __linux__
      pthread_setname_np(thread_, thread_name_.c_str());
#else
      // Other POSIX
      pthread_setname_np(thread_name_.c_str());
#endif

      
      uint8_t tmp[16];
      
      int n;
      int i;
      int c;
      
      Command tx_cmd;
      Command rx_cmd;
      SerializedCommand tx_ser;
      SerializedCommand rx_ser;
      
      tx_cmd.type = kIdle;
      tx_cmd.address = 0x00FFFFFF;  // 24-bit address space
      tx_cmd.data32 = 0xAABBCCDD;

      Encoder(tx_cmd, &tx_ser);

      watch_.Reset();
      watch_.Start();

      c = 0;
      
      while (1) {
            if (thread_exit_) {
                  break;
            }

            // Transmission of kIdle commands every kIdleLinkPeriod just to notify that the serial link is not broken
            if (watch_.ElapsedMilliseconds() > kIdleLinkPeriod) {
                  watch_.Reset();
                  n = RobustWR(fd_, tx_ser.data, tx_ser.size, 50);
                  if (n == 0) {
                        // Timeout
                  } else {
                        
                        printf("Tx Encoded Frame: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
                               tx_ser.data[0], tx_ser.data[1], tx_ser.data[2], tx_ser.data[3], tx_ser.data[4],
                               tx_ser.data[5], tx_ser.data[6], tx_ser.data[7], tx_ser.data[8], tx_ser.data[9]);
                  }
            }

            pthread_mutex_lock(&lock_);
            if (tx_command_valid_) {
                  Encoder(tx_command_, &tx_ser);
                  n = RobustWR(fd_, tx_ser.data, tx_ser.size, 50);
                  if (n != tx_ser.size) {
                        // Tx Failed! What now?
                  }
                  tx_command_valid_ = false;
            }
            pthread_mutex_unlock(&lock_);




            
            
            
            if (rx_command_valid_ == false) {
                  // New Frame Detector
                  n = RobustRD(fd_, tmp, sizeof(tmp), 50);
                  if (n > 0) {

                        //printf("We have received something (%d characters)!\n", n);
                        for (i = 0; i < n; i++) {
                              if ((tmp[i] & 0x80) == 0x80) {
                                    c = 0;
                              }
                        
                              rx_ser.data[c] = tmp[i];
                              c++;
                              rx_ser.size = c;                        
                        
                              if (c == 10) {
                                    if ((rx_ser.data[0] & 0x80) == 0x80) {

                                          printf("Rx Encoded Frame: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
                                                 rx_ser.data[0], rx_ser.data[1], rx_ser.data[2], rx_ser.data[3], rx_ser.data[4],
                                                 rx_ser.data[5], rx_ser.data[6], rx_ser.data[7], rx_ser.data[8], rx_ser.data[9]);
                                    
                                          Decoder(&rx_cmd, rx_ser);

                                          if (rx_cmd.type == kInterrupt) {
                                                if (func_ != nullptr) {
                                                      func_(rx_cmd.address & 0x0000FFFF, rx_cmd.data32);
                                                }
                                          } else {

                                                pthread_mutex_lock(&lock_);
                                                rx_command_ = rx_cmd;
                                                rx_command_valid_ = true;
                                                pthread_mutex_unlock(&lock_);

                                                printf("VALID! :)\n");
                                          }
                                    
                                    }
                                    c = 0;
                              }
                        }
                  
                  }
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
