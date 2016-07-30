// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "fpga_link1.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>


#include <cassert>

using fpga_link1::FpgaLink1;


enum CommandType {
      kRead8     = 0x01,
      kRead16    = 0x02,
      kRead32    = 0x03,
      kWrite8    = 0x04,
      kWrite16   = 0x05,
      kWrite32   = 0x06,
      kInterrupt = 0x07,
      kIdle      = 0x08
};


struct Command {

      // Type of command for the microelectronic system.
      CommandType type;

      // 24-bit address, valid rage is 0x00000000 to 0x00FFFFFF.
      uint32_t address;

      // Data to write to or read from a microelectronic system register.
      union {
	    uint8_t data8;
	    uint16_t data16;
	    uint32_t data32;
      };

            
};


struct SerializedCommand {
      uint8_t data[10];
      int size;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  bit 69              64 63               40 39                              8 7                            0
      +------------------------------------------------------------------------------------------------------+
      | OPCODE (6 bits) | ADDRESS (24 bits) | DATA (32 bits)                  | CRC (8 bits)                 |
      +------------------------------------------------------------------------------------------------------+
 */
int Encoder(Command& cmd, SerializedCommand* serialized) {

      uint8_t tmp[9];
      
      switch (cmd.type) {
      case kWrite8:
	    tmp[0] = cmd.type << 4;
	    tmp[1] = (cmd.address & 0x00ff0000) >> 16;
	    tmp[2] = (cmd.address & 0x0000ff00) >> 8;
	    tmp[3] = (cmd.address & 0x000000ff) >> 0;
	    tmp[4] = cmd.data8;
	    tmp[5] = 0x00;
	    tmp[6] = 0x00;
	    tmp[7] = 0x00;
	    tmp[5] = 0xFF; // CRC

	    break;
      default:
	    assert(false);
      }

      serialized->size = 10;
      
      // First transmitted byte over serial link is serialized->data[0]
      
      serialized->data[9] = ((tmp[8] & 0b01111111) << 0) | ((tmp[0] & 0b00000000) >> 0);  // 7+0
      serialized->data[8] = ((tmp[7] & 0b00111111) << 1) | ((tmp[8] & 0b10000000) >> 7);  // 6+1
      serialized->data[7] = ((tmp[6] & 0b00011111) << 2) | ((tmp[7] & 0b11000000) >> 6);  // 5+2
      serialized->data[6] = ((tmp[5] & 0b00001111) << 3) | ((tmp[6] & 0b11100000) >> 5);  // 4+3
      serialized->data[5] = ((tmp[4] & 0b00000111) << 4) | ((tmp[5] & 0b11110000) >> 4);  // 3+4
      serialized->data[4] = ((tmp[3] & 0b00000011) << 5) | ((tmp[4] & 0b11111000) >> 3);  // 2+5
      serialized->data[3] = ((tmp[2] & 0b00000001) << 6) | ((tmp[3] & 0b11111100) >> 2);  // 1+6
      serialized->data[2] = ((tmp[1] & 0b00000000) << 7) | ((tmp[2] & 0b11111110) >> 1);  // 0+7
      serialized->data[1] = ((tmp[1] & 0b01111111) << 0) | ((tmp[2] & 0b00000000) >> 0);  // 7+0
      serialized->data[0] = ((tmp[0] & 0b00111111) << 1) | ((tmp[1] & 0b10000000) >> 7);  // 6+1
            
      return 0;
}

int Decoder(Command& cmd, SerializedCommand* serialized) {
      return 0;
}



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
      cmd.address = static_cast<uint32_t>(reg);
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
