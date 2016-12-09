// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_FRAMER_H_
#define FPGA_LINK1_FRAMER_H_

#include <pthread.h>
#include <stdint.h>
#include <string>

#include "stopwatch.h"
#include "pollable_sync_queue.h"

namespace fpga_link1 {

      /**
       * Sends and receives 70 bit frames thru serial port.
       */
      class Framer {
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
                  
                  // Input/output error, for example: during read(), write(), poll(), etc, operations. Comunications.
                  IO,

                  // The Enqueue() or Dequeue() operation has returned with timeout.
                  Timeout,

                  // Other error.
                  Generic
            };


            /**
             * The Frame.
             *
             * Do not append a CRC to the payload, this framer already append an 8-bit CRC to the sent frame
             * payload. Please note that from data[0] only the 6 least significant bits are sent and hence 62 bits are
             * sent (not 64).
             */
            struct FixedFrame {
                  // 62-bit frame (in data[0] only 6 LSB are sent, data[0] & 0x3f)
                  uint8_t data[8];
            };

            
            /**
             * Ctor.
             *
             * @param device The serial port device file, it will be setup <SPEED>-8N1
             * @param speed_bps Serial port speed in bits per second, use standard values like 115200, 38400, 9600, etc.
             */
            Framer(std::string device, int speed_bps);
                        
            /**
             * Dtor.
             */
            ~Framer();

            /**
             * One-time (heavy) initialization.
             */
            Error Init();

            /**
             * Enqueue a frame of 70 bits, here |data| must be an array of 9 bytes, in data[0] only 6 LSB are sent. This
             * function is blocking if after |timeout_ms| frame can not be enqueued then it return kErrorTimeout. If
             * |timeout_ms| is 0 then returns inmediattely, if is negative an infinite wait will be done.
             * 
             */
            Error TxQueueEnqueue(FixedFrame& f, int timeout_ms);
            int   TxQueueFileDescriptor();
            int   TxQueueSize();
            int   TxQueueCapacity();

            
            Error RxQueueDequeue(FixedFrame* f, int timeout_ms);
            int   RxQueueFileDescriptor();
            int   RxQueueSize();
            int   RxQueueCapacity();
            
      private:

            static unsigned char crc8_table[];
            static const int kResponsePollPeriod = 2000;       // us, 2000 us = 2 ms
      
            struct SerializedFixedFrame {
                  char data[10];  //+crc
            };

            pthread_t thread_;
            pthread_attr_t thread_attr_;
            pthread_mutex_t lock_;
            bool thread_exit_;
            
            PollableSyncQueue<FixedFrame> tx_queue_;
            PollableSyncQueue<FixedFrame> rx_queue_;
                        
            bool initialized_;
            std::string device_;
            int speed_;
            int fd_;

            FixedFrame txf_;
            FixedFrame rxf_;    
            SerializedFixedFrame txs_;           
            SerializedFixedFrame rxs_;
            int txs_sent_;
            int rxs_size_;
            bool found_;
                        
            static 
            void* ThreadFn(void* obj);
            void* ThreadFn();


            int Encoder(FixedFrame& f, SerializedFixedFrame* s);
            int Decoder(FixedFrame* f, SerializedFixedFrame& s);

            
            int Push(uint8_t c);
            uint8_t Crc8(uint8_t crc, uint8_t *data, int len);
            
      };


      

}


#endif  // FPGALINK1_FRAMER_H_
