/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#include <stdlib.h>

#include <InsertReversedStressTestEventListener.hpp>

int main(void)
{
 InsertReversedStressTestEventListener test;
 
 /* Run the system proper. */
 EventListenerManager::getInstance().run(); 
 return EXIT_SUCCESS;
}
