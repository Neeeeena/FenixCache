/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

# ifndef CACHETESTEVENTLISTENER_HPP
# define CACHETESTEVENTLISTENER_HPP

# include <assert.h>
# include <stdint.h>
# include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <Cache.hpp>
#include <OpPair.hpp>

# include <EventListener.hpp>

# include <EventListenerManager.hpp>
# include <UUID.hpp>
# include <FileSystemManager.hpp>
# include <TransactionManager.hpp>
# include <FileSystem.hpp>
# include <SubTreeTransaction.hpp>
# include <SubTreeBlobKey.hpp>

using namespace std;

class CacheTestEventListener: public EventListener{


public:
  inline
  CacheTestEventListener()
  {
  tracefile.open("files/resourcetrace1.txt");
   traceLine = 0;
   register enum EventListenerManager::EventListenerManagerError
   error;

   if (!EventListenerManager::getInstance().registerListener(error, this, __func__))
   {
    assert(0);
   }

   assert(error == EventListenerManager::noError);
  }


	OpPair ReadTraceLine(){

		string line;
		getline(tracefile, line);

		istringstream iss(line);
		stringstream ss(line);
		string token;
		getline(ss,token,',');
		string operation = trim(token);
		//evt bruge
		getline(ss,token,',');
		getline(ss,token,',');
		//gfid
		getline(ss,token,',');
		string id = trim(token);
		int gid = extractID(id);

		return (OpPair){operation,gid};

	}


  inline virtual bool
  handleEvent(register unsigned int&            receiver,
              register class Event*&            outgoingEvent,
              register const unsigned int       sender,
              register const class Event* const incomingEvent)
  {

   /* Consume event when done with it. */
   delete incomingEvent;
   traceLine++;
   
   OpPair treeOp;
		treeOp = ReadTraceLine();
		if(treeOp.Operation == "OPEN"){
			cout << "Open file: " << treeOp.ID <<"\n";
		}else if(treeOp.Operation == "SREAD"){
			cout << "Read from file " << treeOp.ID << "\n";
		}else if(treeOp.Operation == "SWRITE"){
			cout << "Write to file " << treeOp.ID << "\n";
		}else if(treeOp.Operation == "CLOSE"){
			cout << "Close file " << treeOp.ID << "\n";
		}else if(treeOp.Operation == "UNLINK"){
			cout << "Delete file " << treeOp.ID << "\n";
		}else if(treeOp.Operation == "LINK"){
			cout << "Link file " << treeOp.ID << "\n";
		}
		else{
			cout << "What is that!" << "\n";
		}
		return 0;


    if(traceLine == 10){
        return false;
    }

  register enum EventListenerManager::EventListenerManagerError
   error;

   if (!EventListenerManager::getInstance().deRegisterListener(error, this))
   {
    assert(0);
   }

   assert(error == EventListenerManager::noError);

   return false;
}

private:
	ifstream tracefile;
    int traceLine;

  int extractID(string& str)
  {
      size_t first = str.find_first_of("=");
      return atoi(str.erase(0,(first+1)).c_str());
  }

  string trim(string& str)
  {
      size_t first = str.find_first_not_of(' ');
      size_t last = str.find_last_not_of(' ');
      return str.substr(first, (last-first+1));
  }

};

#endif
