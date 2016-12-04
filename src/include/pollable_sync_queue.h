// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_POLLABLE_SYNC_QUEUE_H_
#define FPGA_LINK1_POLLABLE_SYNC_QUEUE_H_

#include <unistd.h>
#include <pthread.h>
#include <queue>
#include <cassert>

namespace fpga_link1 {

/**
 * Cola síncrona.
 * Utilización:
 * 
 * PollableSyncQueue<int> q(5);
 *
 * q.Push(1);
 * q.Push(3);
 * q.Push(7);
 *
 * q.Size();        // 3
 * q.Capacity();    // 5
 * 
 * q.Push(9);
 * q.Push(10);
 * q.Push(21);      // Aqui este hilo queda bloqueado a la espera de que la cola se vacie desde otro hilo.
 *                  // Si nadie extrajese algun elemento sufririamos un bloqueo indefinido (deadlock)
 *
 * (supongamos que algun hilo extrae algun elemento ==> el programa continua...)
 * 
 * int x;
 * 
 * x = q.Front();   // x vale 1
 * x = q.Front();   // x vale 1
 *
 * q.Pop();
 * x = q.Front();   // x vale 3
 * x = q.Front();   // x vale 3
 *
 * q.Pop();
 * x = q.Front();   // x vale 7
 * 
 * q.Pop();   
 * x = q.Front();   // x vale 9
 * q.Pop();  
 * q.Pop();  
 * q.Pop();         // La cola está vacía por lo que aquí este hilo queda bloqueado a la espera de que algún 
 *                  // hilo ponga un elemento en la cola 
 *
 */
template <class T> 
class PollableSyncQueue {        
public:

      /**
       * @param capacity Maximun number of elements enqueued
       */
      PollableSyncQueue(int capacity); 

      ~PollableSyncQueue(); 
      
      /** Aade un elemento a la cola */
      int  Push(const T& val);  

      /** Devuelve el primer elemento de la cola, pero no lo extrae */
      T&   Front(); 

      /** Extrae el primer elemento de la cola */
      void Pop();

      /** Nmero de elementos en la cola */
      int  Size();

      /** Capacidad de la cola (nmero mximo de elementos que puede albergar) */
      int  Capacity();


      int RoFileDescriptor();
      int WoFileDescriptor();
      
private:
      int             m_fd[2];
      std::queue<T>*  m_queue;
      int             m_capacity;
      int             m_count;
      pthread_mutex_t m_lock;
      pthread_cond_t  m_cond_space; // variable de condicin sobre la que suspenderse si no hay espacio
      pthread_cond_t  m_cond_items; // variable de condicin sobre la que suspenderse si no hay elementos

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> PollableSyncQueue<T>::PollableSyncQueue(int capacity) 
{
      m_capacity = capacity;
      m_count = 0;

      pthread_mutex_init(&m_lock, NULL);
      pthread_cond_init(&m_cond_space, NULL);
      pthread_cond_init(&m_cond_items, NULL);

      m_queue = new std::queue<T>();

      assert(pipe(m_fd) == 0);      
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> PollableSyncQueue<T>::~PollableSyncQueue() 
{
      close(m_fd[0]);
      close(m_fd[1]);
      m_fd[0] = -1;
      m_fd[1] = -1;
      delete m_queue; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      template<class T> int PollableSyncQueue<T>::RoFileDescriptor() {
            return m_fd[0];
      }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      template<class T> int PollableSyncQueue<T>::WoFileDescriptor() {
            return m_fd[1];
      }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> int PollableSyncQueue<T>::Push(const T& val)
{
      pthread_mutex_lock(&m_lock);

      while (m_count == m_capacity)
      {
	    pthread_cond_wait(&m_cond_space, &m_lock); 
      }

      char c;      
      assert(write(m_fd[1], &c, 1) == 1);
      
      m_queue->push(val);  	
      m_count++;
      pthread_cond_signal(&m_cond_items);

      pthread_mutex_unlock(&m_lock);    
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> T& PollableSyncQueue<T>::Front() 
{
      pthread_mutex_lock(&m_lock);

      while (m_count == 0)
      {
	    pthread_cond_wait(&m_cond_items, &m_lock); 
      }
      
      T& val = m_queue->front();
      
      pthread_mutex_unlock(&m_lock);    

      return val; 
}              


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> void PollableSyncQueue<T>::Pop()
{
      pthread_mutex_lock(&m_lock);

      while (m_count == 0)
      {
	    pthread_cond_wait(&m_cond_items, &m_lock); 
      }

      char c;      
      assert(read(m_fd[0], &c, 1) == 1);
      
      m_queue->pop();
      m_count--;
      pthread_cond_signal(&m_cond_space);

      pthread_mutex_unlock(&m_lock);    
}


  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> int PollableSyncQueue<T>::Size() 
{
      unsigned int sz;

      pthread_mutex_lock(&m_lock);

      sz = m_queue->size();

      pthread_mutex_unlock(&m_lock);

      return sz;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> int PollableSyncQueue<T>::Capacity() 
{
      return m_capacity;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // librest

#endif   // FPGA_LINK1_POLLABLE_SYNC_QUEUE_H_
