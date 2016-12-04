// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_SERVER_H_
#define FPGA_LINK1_SERVER_H_

#include <pthread.h>
#include <stdint.h>
#include <string>

#include "stopwatch.h"
#include "codec.h"
#include "framer.h"


namespace fpga_link1 {

      typedef void (*InterruptCallback)(uint16_t interrupt_number, uint32_t code);
      
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
            
            enum class Error {
                  // No error.
                  No,
                 
                  // The specified device does not exist.
                  NoSuchDevice,

                  // Error while creating a the internal thread (with pthread_create).
                  ThreadCreation,
                  
                  // Error during calling tcgetattr(), settcaatr() and other <termios.h> apis
                  Termios,

                  // One or more parameters are wrong, for example: a data buffer pointer is null or data buffer size is
                  // too large (outside the permited range).
                  WrongParameters,

                  // Device has informed that the requested read or write operation was unsuccesful (nack)
                  OperationNotAcknowledged,

                  // Timeout, device did not send a response.
                  Timeout,
                        
                  // Input/output error, for example: during read(), write(), poll(), etc, operations. Comunications.
                  IO,

                  // Protocol error, device has sent an unexpected frame which does not correspond with the sent one.
                  Protocol,

                  // Other, not specified, error.
                  Generic
            };


            /**
             * Ctor.
             *
             * @param device The serial port device file, it will be setup <SPEED>-8N1
             * @param speed_bps Serial port speed in bits per second, use standard values like 115200, 38400, 9600, etc.
             */
            FpgaLink1(std::string device, int speed_bps);
            
            
            /**
             * Dtor.
             */
            ~FpgaLink1();

            /**
             * One-time (heavy) initialization.
             */
            Error Init();

            Error RegisterInterruptCallback(InterruptCallback f);
            
            Error MemoryRD32(uint32_t address, uint32_t* data, int timeout_ms);
            Error MemoryWR32(uint32_t address, uint32_t  data, int timeout_ms);

      private:

            static const int kResponsePollPeriod = 2000;       // us, 2000 us = 2 ms
            static const int kIdleSleep = 1000;                // us, 1000 us = 1 ms
            static const int kIdleLinkPeriod = 1000;           // ms
            
            bool initialized_;
            pthread_t thread_;
            pthread_attr_t thread_attr_;
            pthread_mutex_t lock_;
            std::string thread_name_;
            bool thread_exit_;

            Framer framer_;
            
            Stopwatch watch_;
            InterruptCallback func_;

            // Send and receive buffers, _valid variables indicates that the corresponding buffers are not empty.
            // Rx buffer, capacity = 1 command
            Frame rx_command_;
            bool    rx_command_valid_;

            // Tx buffer, capacity = 1 command
            Frame tx_command_;
            bool    tx_command_valid_;
            
            static 
            void* ThreadFn(void* obj);
            void* ThreadFn();

            int RobustWR(int fd, uint8_t* s, int n, int timeout_ms);
            int RobustRD(int fd, uint8_t* s, int n, int timeout_ms);



      };

}


#endif  // FPGALINK1_SERVER_H_
