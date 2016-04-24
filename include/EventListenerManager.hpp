/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#ifndef EVENTLISTENERMANAGER_HPP
# define EVENTLISTENERMANAGER_HPP

# include <assert.h>
# include <string.h>

# include <EventListener.hpp>
# include <Event.hpp>

# include <TickEvent.hpp>

class EventListenerManager
{
 public:
  enum EventListenerManagerError
  {
   noError = 0,
   tooManyListeners,
   noListener
  };

  static inline EventListenerManager& 
  getInstance(void)
  {
   return instance;
  }

  /* WARNING: sendEvent can lead to circular event dependencies if you
   * are not carefull! */
  inline bool
  sendEvent(register enum EventListenerManagerError& error,
            register unsigned int                    receiver,
            register class Event*                    event)
  {
   register unsigned int sender = currentListener;
   register bool         moreEvents;

   do
   {
    assert(receiver < maxListeners);

    assert(listeners[receiver].valid);
    assert(listeners[receiver].listener);

    register unsigned int newReceiver;
    register class Event* newEvent;

    currentListener = receiver;

    moreEvents = listeners[receiver].listener->handleEvent(newReceiver, newEvent, sender, event);
       
    sender   = receiver;
    event    = newEvent;
    receiver = newReceiver;

   } while (moreEvents);
   
   error = noError;
   return true;
  }

  inline bool
  registerListener(register enum EventListenerManagerError& error,
                   register class EventListener* const      listener,
                   register const char* const               name)
  {
   for(register unsigned int i = 0; i < maxListeners; i++)
   {
    if (!listeners[i].valid)
    {
     assert(listener);
     assert(name);

     listeners[i].listener = listener;
     listeners[i].name = name;
     listeners[i].valid = true;

     error = noError;
     return true;
    }
   }

   error = tooManyListeners;
   return false;   
  }

  inline bool
  find(register enum EventListenerManagerError& error,
       register unsigned int&                   id,
       register const char* const               name)
  {
   for(register unsigned int i = 0; i < maxListeners; i++)
   {
    if (listeners[i].valid)
    {
     assert(listeners[i].name);
     assert(name);

     if (!strcmp(listeners[i].name, name))
     {
      id = i;

      error = noError;
      return true;
     }
    }
   }

   error = noListener;
   return false;   
  }


  inline bool
  deRegisterListener(register enum EventListenerManagerError&  error,
                     register const class EventListener* const listener)
  {
   for(register unsigned int i = 0; i < maxListeners; i++)
   {
    if (listeners[i].valid)
    {
     assert(listener);

     if (listeners[i].listener == listener)
     {
      listeners[i].valid = false;

      error = noError;
      return true;
     }
    }
   }

   error = noListener;
   return false;   
  }

  inline void
  run(void)
  {
   register bool listenersAlive;
   do
   {
    listenersAlive = false;
    /*! \todo make this more elaborate. */
    for(register unsigned int i = 0; i < maxListeners; i++)
    {
     currentListener = maxListeners;

     if (listeners[i].valid)
     {
      register enum EventListenerManagerError
      error;

      listenersAlive = true;

      if (!sendEvent(error , i, new TickEvent))
      {
       assert(0);
      }
     
      assert(error == noError);
     }
    }
   } while (listenersAlive);
  }

 private:
  static const unsigned int
  maxListeners = 16;
  
  static EventListenerManager
  instance;

  unsigned int
  currentListener;

  struct
  {
   bool                 valid;
   class EventListener* listener;
   const char*          name;
  } listeners[maxListeners];

  inline
  EventListenerManager()
  {
   for(register unsigned int i = 0; i < maxListeners; i++)
    listeners[i].valid = false;
  }
};

#endif
