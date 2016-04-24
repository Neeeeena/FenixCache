/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#ifndef EVENTLISTENER_HPP
# define EVENTLISTENER_HPP

# include <assert.h>

class EventListener
{
 public:
  /*! \returns true if outgoingEvent is to be sent. */
  virtual bool
  handleEvent(register unsigned int&            receiver,
              register class Event*&            outgoingEvent,
              register const unsigned int       sender,
              register const class Event* const incomingEvent) = 0;
};

#endif
