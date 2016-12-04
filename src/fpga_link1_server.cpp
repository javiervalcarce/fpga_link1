// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "fpga_link1_server.h"
// platform
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
// c++
#include <cassert>
#include <cstdio>
#include <cstring>
// app
#include "codec.h"

using fpga_link1::FpgaLink1Server;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1Server::FpgaLink1Server(std::string device, int speed_bps) : framer_(device, speed_bps) {
      thread_exit_ = false;
      initialized_ = false;
      func_ = nullptr;
      
      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);  
      pthread_mutex_init(&lock_, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1Server::~FpgaLink1Server() {
      thread_exit_ = true;
      pthread_join(thread_, NULL);
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FpgaLink1Server::Error FpgaLink1Server::Init() {
      int r;

      framer_.Init();
      
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

      watch_.Reset();
      watch_.Start();
     
      Frame           reqf;  // Request frame
      SerializedFrame reqs;  // Request frane serialized
      Frame           resf;  // Response frame
      SerializedFrame ress;  // Response frame serialized

      Framer::FixedFrame tx;
      Framer::FixedFrame rx;
      Framer::Error e;
      
      while (1) {

            if (thread_exit_) {
                  break;
            }

            e = framer_.RxQueueDequeue(&rx, 500);
            if (e == Framer::Error::Timeout) {
                  printf("RxQueue timeout\n");
                  continue;
            }
            if (e != Framer::Error::No) {
                  printf("RxQueue error\n");
            }


            memcpy(reqs.data, rx.data, 8);
            Decoder(&reqf, reqs);
            
            if (reqf.type != kPing) {
                  if (func_ != nullptr) {
                        func_(reqf.type, reqf.address, &reqf.data32);
                  }
            }
            
            switch (reqf.type) {
            case kPing: resf.type = kPingAck; break;
            case kRead32: resf.type = kRead32Ack; break;
            case kWrite32: resf.type = kWrite32Ack; break;
            default:
                  // Comando desconocido.
                  continue;
                  break;
            }
            resf.address = reqf.address;
            resf.data32  = reqf.data32;
                  
            // Frame Encoder
            Encoder(resf, &ress);
            memcpy(tx.data, ress.data, 8);
            
            e = framer_.TxQueueEnqueue(tx, 500);
            if (e == Framer::Error::Timeout) {
                  printf("TxQueue timeout\n");
                  continue;
            }
            if (e != Framer::Error::No) {
                  printf("TxQueue error\n");
            }
            
            
      }  // while (1)

      return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FpgaLink1Server::Error FpgaLink1Server::SendInterrupt(int irq) {
//      return Error::No;
//}

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
