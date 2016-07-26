// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_H_
#define FPGA_LINK1_H_

#include <pthread.h>
#include <stdint.h>

#include <string>



namespace fpga_link1 {

      /**
       * An object of this class lets you send and receive data to an FPGA. At the FPGA side is required a specific
       * hardware to process and respond the commands sent by this software. See vhdl/ directory.
       * 
       * - A serial port UART-8N1 tx and rx blocks. The PC is connected by serial port to the FPGA.
       *
       * - A SLIP Byte Stuffing framer/deframer blocks.
       *
       * - A RMAP (Remote Mameory Access Protocol) block
       *
       * - The system bus can be APB, AHB, Wishbone, etc, this is specified by the digital HW designer and is
       *   transparent to this sofware.
       *
       */
      class FpgaLink1 {
      public:
            enum Error {
                  kErrorNo,
                  kErrorIO
            };


            /**
             * Ctor.
             * @param thread_name The name of service thread, this name will show in a GDB session typing "info
             * threads" command.
             */
            FpgaLink1(std::string device);

            /**
             * Dtor.
             */
            ~FpgaLink1();

            /**
             * One-time (heavy) initialization.
             */
            Error Init();


            Error MemoryRD(int reg, uint8_t* data);
            Error MemoryRD(int reg, uint8_t* data, int len);

            //
            Error MemoryWR(int reg, uint8_t  data);
            Error MemoryWR(int reg, uint8_t* data, int len);

            //
            Error FifoRD(int reg, uint8_t* data, int len);
            Error FifoWR(int reg, uint8_t* data, int len);

      private:

            static const int kIdleSleep = 1000; // 1000 us = 1 ms

            pthread_t thread_;
            pthread_attr_t thread_attr_;

            pthread_mutex_t lock_;
            std::string thread_name_;
            std::string device_;

            bool thread_exit_;

            static 
            void* ThreadFn(void* obj);
            void* ThreadFn();

      };

}


#endif  // FPGALINK1_H_
