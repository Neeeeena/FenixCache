/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#include <stdlib.h>

#include <InsertRemoveReversedStressTestEventListener.hpp>

int main(void)
{
 InsertRemoveReversedStressTestEventListener test;
 
 /* Run the system proper. */
 EventListenerManager::getInstance().run(); 
 return EXIT_SUCCESS;
}
