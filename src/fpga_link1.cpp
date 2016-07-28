#include "fpga_link1.h"
#include <unistd.h>
#include <cassert>

using fpga_link1::FpgaLink1;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::FpgaLink1(std::string device) {
      thread_name_ = "fpga_link1";
      thread_exit_ = false;
      initialized_ = false;
      
      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);  
      pthread_mutex_init(&lock_, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::~FpgaLink1() {
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1::Error FpgaLink1::Init() {
      int r;
      r = pthread_create(&thread_, &thread_attr_, FpgaLink1::ThreadFn, this);
      if (r != 0) {
            return kErrorNo;
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

      int wait_for;
      int r;

      while (1) {

            if (thread_exit_) {
                  break;
            }
            usleep(1e6);
      }

      return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
