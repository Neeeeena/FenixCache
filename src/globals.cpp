/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#include <TransactionManager.hpp>
#include <FileSystemManager.hpp>
#include <OSInterface.hpp>
#include <VirtualBlockDeviceBroker.hpp>
#include <SubTreeObserverManager.hpp>
#include <EventListenerManager.hpp>
#include <SimpleAnalyzerObserver.hpp>
#include <BlockCache.hpp>

/* Static class members. */

SubTreeObserverManager
SubTreeObserverManager::instance;

SimpleAnalyzerObserver
observer;

OSInterface
OSInterface::instance;

BlockCache
BlockCache::instance;

TransactionManager
TransactionManager::instance;

VirtualBlockDeviceBroker
VirtualBlockDeviceBroker::instance;

FileSystemManager
FileSystemManager::instance;

EventListenerManager
EventListenerManager::instance;
