// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "framer.h"
// platform
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>
#include <signal.h>

// c++
#include <cassert>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include "codec.h"

using fpga_link1::Framer;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PrintFrame(std::string prompt, fpga_link1::Framer::FixedFrame f) {
      std::ostringstream os;

      os << prompt;
      
      for (int i = 0; i < 8; ++i) {
            os << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(f.data[i]);
            if (i < 7) {
                  os << "-";
            }
      }
      os << std::endl;

      printf(os.str().c_str());
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Framer::Framer(std::string device, int speed_bps) : tx_queue_(10), rx_queue_(10) {

      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);  
      pthread_mutex_init(&lock_, NULL);
      thread_exit_ = false;
 
      initialized_ = false;
      device_ = device;
      speed_ = speed_bps;
      fd_ = -1;
      txs_sent_ = 0;
      rxs_size_ = 0;
      found_ = false;     
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Framer::~Framer() {
      thread_exit_ = true;
      pthread_join(thread_, NULL);
      // Terminate first the internal thread
      if (fd_ != -1) {
            close(fd_);
      }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Framer::Error Framer::Init() {
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

      r = pthread_create(&thread_, &thread_attr_, Framer::ThreadFn, this);
      if (r != 0) {
            return Error::ThreadCreation;
      }

      initialized_ = true;
      return Error::No;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Framer::Error Framer::TxQueueEnqueue(FixedFrame& f, int timeout_ms) {
      tx_queue_.Push(f);     
      return Error::No;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Framer::TxQueueFileDescriptor() {
      return tx_queue_.WoFileDescriptor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Framer::TxQueueSize() {
      return tx_queue_.Size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Framer::TxQueueCapacity() {      
      return tx_queue_.Capacity();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Framer::Error Framer::RxQueueDequeue(FixedFrame* f, int timeout_ms) {
      *f = rx_queue_.Front();
      rx_queue_.Pop();
      return Error::No;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Framer::RxQueueFileDescriptor() {
      return rx_queue_.RoFileDescriptor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Framer::RxQueueSize() {
      return rx_queue_.Size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Framer::RxQueueCapacity() {
      return rx_queue_.Capacity();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* Framer::ThreadFn(void* obj) {
      Framer* o = reinterpret_cast<Framer*>(obj);
      return o->ThreadFn();
}
void* Framer::ThreadFn() {
     
      // Mask all signals for this thread.
      sigset_t set;
      
      //sigaddset(&set, SIGPIPE);
      //sigfillset(&set);
      //pthread_sigmask(SIG_SETMASK, &set, NULL);
      

      struct pollfd fda[3];
      int n;

      const int TX = 0;
      const int RX = 1;
      const int SP = 2;

      
      // Initial events to listen for
      // read from serial port
      // read from tx queue
      // See http://www.greenend.org.uk/rjk/tech/poll.html
      
      fda[TX].fd      = tx_queue_.RoFileDescriptor();
      fda[TX].events  = POLLIN;
      fda[TX].revents = 0;
      
      fda[RX].fd      = rx_queue_.WoFileDescriptor();
      fda[RX].events  = 0;
      fda[RX].revents = 0;
      
      fda[SP].fd      = fd_;
      fda[SP].events  = POLLIN;
      fda[SP].revents = 0;

      
      while (1) {

            if (thread_exit_) {
                  break;
            }

            //printf("poll()\n");
            
            n = poll(fda, 3, 1000);
            if (n < 0) {
                  printf("poll ERROR\n");
            } else if (n == 0) {
                  //                  printf("poll timeout\n");
            } else {

                  /*
                  printf("n = %d\n", n);

                  
                  printf("TX POLLIN  = %d\n", ((fda[TX].revents & POLLIN ) != 0));
                  printf("TX POLLOUT = %d\n", ((fda[TX].revents & POLLOUT) != 0));
                  printf("TX POLLHUP = %d\n", ((fda[TX].revents & POLLHUP) != 0));
                  printf("TX POLLERR = %d\n", ((fda[TX].revents & POLLERR) != 0));

                  printf("RX POLLIN  = %d\n", ((fda[RX].revents & POLLIN ) != 0));
                  printf("RX POLLOUT = %d\n", ((fda[RX].revents & POLLOUT) != 0));
                  printf("RX POLLHUP = %d\n", ((fda[RX].revents & POLLHUP) != 0));
                  printf("RX POLLERR = %d\n", ((fda[RX].revents & POLLERR) != 0));

                  printf("SP POLLIN  = %d\n", ((fda[SP].revents & POLLIN ) != 0));
                  printf("SP POLLOUT = %d\n", ((fda[SP].revents & POLLOUT) != 0));
                  printf("SP POLLHUP = %d\n", ((fda[SP].revents & POLLHUP) != 0));
                  printf("SP POLLERR = %d\n", ((fda[SP].revents & POLLERR) != 0));
                  */
                  
                  //uint8_t c;
                  //int n = read(fda[TX].fd, &c, 1);
                  //printf("read = %d, %d\n", n, c);
                         
                  
                  if (((fda[TX].revents & (POLLIN | POLLHUP)) == POLLIN)) {
                      
                        // 1 - tx_queue readable (and has at least 1 character because POLLHUP is false).                        
                        txf_ = tx_queue_.Front(); // blocking
                        tx_queue_.Pop();

                        PrintFrame("Tx frame: ", txf_);
                        
                        Framer::Encoder(txf_, &txs_);
                        txs_sent_ = 0;

                        
                              
                        fda[TX].events &= ~POLLIN;
                        fda[SP].events |= POLLOUT;
                  }

                  
                  if ((fda[SP].revents & POLLOUT) == POLLOUT) {
                        // 2 - serial port writable
                        //printf("sp writable\n");
                        assert(write(fd_, txs_.data + txs_sent_, 1) == 1);
                        txs_sent_++;

                        if (txs_sent_ == 10) {
                              // Ya no espero por la escritura del puerto serie.
                              fda[TX].events |= POLLIN;
                              fda[SP].events &= ~POLLOUT;
                        }
                  }

                  
                  if ((fda[SP].revents & POLLIN) == POLLIN) {
                        // 3 - serial port readable (and has at least one character).
                        //printf("sp readable\n");
                        uint8_t c;
                        n = read(fd_, &c, 1);
                        Push(c);
                        
                        if (found_) {
                              if (Decoder(&rxf_, rxs_) == 0) {
                                                                        
                                    fda[RX].events |= POLLOUT;
                                    fda[SP].events &= ~POLLIN;
                              } else {
                                    found_ = false;
                              }
                        }
                  }
                  
                  if ((fda[RX].revents & POLLOUT) == POLLOUT) {
                        // 4 - rx_queue writable
                        //printf("rxq writable\n");
                        if (found_) {
                              found_ = false;
                              rx_queue_.Push(rxf_);  // BLOCKING!?

                              
                              PrintFrame("Rx frame: ", rxf_);
                        }
                        
                        fda[RX].events &= ~POLLOUT;
                        fda[SP].events |= POLLIN;
                  }            
            }  

            
      }  // while (1)


      return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Framer::Encoder(FixedFrame& f, SerializedFixedFrame* s) {
      assert(initialized_ == true);
      uint8_t tmp[9];

      // TODO: enhance this
      tmp[0] = f.data[0];
      tmp[1] = f.data[1];
      tmp[2] = f.data[2];
      tmp[3] = f.data[3]; 
      tmp[4] = f.data[4];
      tmp[5] = f.data[5];
      tmp[6] = f.data[6];
      tmp[7] = f.data[7];
      tmp[8] = Crc8(0xFF, tmp, 8);
  
      
      // First transmitted byte over serial link is s->data[0]      
      s->data[9] = ((tmp[8] & 0b01111111) << 0) | ((tmp[0] & 0b00000000) >> 0);  // 7+0
      s->data[8] = ((tmp[7] & 0b00111111) << 1) | ((tmp[8] & 0b10000000) >> 7);  // 6+1
      s->data[7] = ((tmp[6] & 0b00011111) << 2) | ((tmp[7] & 0b11000000) >> 6);  // 5+2
      s->data[6] = ((tmp[5] & 0b00001111) << 3) | ((tmp[6] & 0b11100000) >> 5);  // 4+3
      s->data[5] = ((tmp[4] & 0b00000111) << 4) | ((tmp[5] & 0b11110000) >> 4);  // 3+4
      s->data[4] = ((tmp[3] & 0b00000011) << 5) | ((tmp[4] & 0b11111000) >> 3);  // 2+5
      s->data[3] = ((tmp[2] & 0b00000001) << 6) | ((tmp[3] & 0b11111100) >> 2);  // 1+6
      s->data[2] = ((tmp[1] & 0b00000000) << 7) | ((tmp[2] & 0b11111110) >> 1);  // 0+7      
      s->data[1] = ((tmp[1] & 0b01111111) << 0) | ((tmp[2] & 0b00000000) >> 0);  // 7+0
      s->data[0] = ((tmp[0] & 0b00111111) << 1) | ((tmp[1] & 0b10000000) >> 7);  // 6+1

      // First byte of encoded frame has its MSB set to '1', all others set to '0'.
      s->data[0] |= 0b10000000;
      /*
        printf("Encoder: Encoded Frame    : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
        s->data[0],
        s->data[1],
        s->data[2],
        s->data[3],
        s->data[4],
        s->data[5],
        s->data[6],
        s->data[7],
        s->data[8],
        s->data[9]);
      */
      
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Framer::Decoder(FixedFrame* f, SerializedFixedFrame& s) {
      assert(initialized_ == true);
      uint8_t tmp[9];

      /*
        printf("Decoder: Encoded Frame    : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
        s.data[0],
        s.data[1],
        s.data[2],
        s.data[3],
        s.data[4],
        s.data[5],
        s.data[6],
        s.data[7],
        s.data[8],
        s.data[9]);
      */
            
      tmp[8] = ((s.data[8] & 0b00000001) << 7) | ((s.data[9] & 0b01111111) >> 0);
      tmp[7] = ((s.data[7] & 0b00000011) << 6) | ((s.data[8] & 0b01111110) >> 1);
      tmp[6] = ((s.data[6] & 0b00000111) << 5) | ((s.data[7] & 0b01111100) >> 2);
      tmp[5] = ((s.data[5] & 0b00001111) << 4) | ((s.data[6] & 0b01111000) >> 3);
      tmp[4] = ((s.data[4] & 0b00011111) << 3) | ((s.data[5] & 0b01110000) >> 4);
      tmp[3] = ((s.data[3] & 0b00111111) << 2) | ((s.data[4] & 0b01100000) >> 5);      
      tmp[2] = ((s.data[2] & 0b01111111) << 1) | ((s.data[3] & 0b01000000) >> 6);
      tmp[1] = ((s.data[0] & 0b00000001) << 7) | ((s.data[1] & 0b01111111) >> 0);
      tmp[0] = ((s.data[0] & 0b00111110) >> 1);
      

      f->data[0] = tmp[0];
      f->data[1] = tmp[1];
      f->data[2] = tmp[2];
      f->data[3] = tmp[3];
      f->data[4] = tmp[4];
      f->data[5] = tmp[5];
      f->data[6] = tmp[6];
      f->data[7] = tmp[7];

      /*
      // tmp[8] CRC      
      if (tmp[8] != Crc8(0xFF, tmp, 8)) {
            // Bad CRC.
            return 1;
      }
      */
      
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Framer::Push(uint8_t c) {
      assert(initialized_ == true);

      printf("RXC = 0x%02X\n", (int) c);
      
      if (found_) {
            return 1;
      }

      if ((c & 0x80) == 0x80) {
            rxs_size_ = 0;
      }

      rxs_.data[rxs_size_] = c;            
      rxs_size_++;
                    
      if (rxs_size_ == 10) {
            if ((rxs_.data[0] & 0x80) == 0x80) {

                  // TODO: Comprobar el CRC
                  //crc = Crc8(0xFF, rxs_.data, 10);
                   
                  found_ = true;
            }
            
            rxs_size_ = 0;
      }
            
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* 8-bit CRC with polynomial x^8+x^6+x^3+x^2+1, 0x14D.
   Chosen based on Koopman, et al. (0xA6 in his notation = 0x14D >> 1):
   http://www.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf
*/

unsigned char Framer::crc8_table[] = {
      0x00, 0x3e, 0x7c, 0x42, 0xf8, 0xc6, 0x84, 0xba, 0x95, 0xab, 0xe9, 0xd7,
      0x6d, 0x53, 0x11, 0x2f, 0x4f, 0x71, 0x33, 0x0d, 0xb7, 0x89, 0xcb, 0xf5,
      0xda, 0xe4, 0xa6, 0x98, 0x22, 0x1c, 0x5e, 0x60, 0x9e, 0xa0, 0xe2, 0xdc,
      0x66, 0x58, 0x1a, 0x24, 0x0b, 0x35, 0x77, 0x49, 0xf3, 0xcd, 0x8f, 0xb1,
      0xd1, 0xef, 0xad, 0x93, 0x29, 0x17, 0x55, 0x6b, 0x44, 0x7a, 0x38, 0x06,
      0xbc, 0x82, 0xc0, 0xfe, 0x59, 0x67, 0x25, 0x1b, 0xa1, 0x9f, 0xdd, 0xe3,
      0xcc, 0xf2, 0xb0, 0x8e, 0x34, 0x0a, 0x48, 0x76, 0x16, 0x28, 0x6a, 0x54,
      0xee, 0xd0, 0x92, 0xac, 0x83, 0xbd, 0xff, 0xc1, 0x7b, 0x45, 0x07, 0x39,
      0xc7, 0xf9, 0xbb, 0x85, 0x3f, 0x01, 0x43, 0x7d, 0x52, 0x6c, 0x2e, 0x10,
      0xaa, 0x94, 0xd6, 0xe8, 0x88, 0xb6, 0xf4, 0xca, 0x70, 0x4e, 0x0c, 0x32,
      0x1d, 0x23, 0x61, 0x5f, 0xe5, 0xdb, 0x99, 0xa7, 0xb2, 0x8c, 0xce, 0xf0,
      0x4a, 0x74, 0x36, 0x08, 0x27, 0x19, 0x5b, 0x65, 0xdf, 0xe1, 0xa3, 0x9d,
      0xfd, 0xc3, 0x81, 0xbf, 0x05, 0x3b, 0x79, 0x47, 0x68, 0x56, 0x14, 0x2a,
      0x90, 0xae, 0xec, 0xd2, 0x2c, 0x12, 0x50, 0x6e, 0xd4, 0xea, 0xa8, 0x96,
      0xb9, 0x87, 0xc5, 0xfb, 0x41, 0x7f, 0x3d, 0x03, 0x63, 0x5d, 0x1f, 0x21,
      0x9b, 0xa5, 0xe7, 0xd9, 0xf6, 0xc8, 0x8a, 0xb4, 0x0e, 0x30, 0x72, 0x4c,
      0xeb, 0xd5, 0x97, 0xa9, 0x13, 0x2d, 0x6f, 0x51, 0x7e, 0x40, 0x02, 0x3c,
      0x86, 0xb8, 0xfa, 0xc4, 0xa4, 0x9a, 0xd8, 0xe6, 0x5c, 0x62, 0x20, 0x1e,
      0x31, 0x0f, 0x4d, 0x73, 0xc9, 0xf7, 0xb5, 0x8b, 0x75, 0x4b, 0x09, 0x37,
      0x8d, 0xb3, 0xf1, 0xcf, 0xe0, 0xde, 0x9c, 0xa2, 0x18, 0x26, 0x64, 0x5a,
      0x3a, 0x04, 0x46, 0x78, 0xc2, 0xfc, 0xbe, 0x80, 0xaf, 0x91, 0xd3, 0xed,
      0x57, 0x69, 0x2b, 0x15};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t Framer::Crc8(uint8_t crc, uint8_t *data, int len) {
      uint8_t* end;

      if (len == 0) {
            return crc;
      }
    
      crc ^= 0xff;
      end = data + len;
      do {
            crc = crc8_table[crc ^ *data++];
      } while (data < end);
    
      return crc ^ 0xff;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
