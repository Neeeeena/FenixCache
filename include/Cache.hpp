 /*
 * Cache.hpp
 *
 *  Created on: 11/05/2016
 *      Author: Nina
 */

#ifndef INCLUDES_CACHE_HPP_
#define INCLUDES_CACHE_HPP_


#include <list>
using namespace std;

class Cache{
public:
	Cache(int numberOfSlots){
		n =numberOfSlots;
		const size_t fixedListSize(n);
		buffer = list<int>(fixedListSize);
	}

	void insert(int ID){
		if(buffer.size()==n){
			buffer.pop_front();
		}
		buffer.push_back(ID);
	}

	bool isInCache(int ID){
		for(auto it = buffer.begin(); it!=buffer.end();++it){
			if(*it == ID){
				//push to the most recently accessed spot
				buffer.remove(*it);
				buffer.push_back(*it);
				return true;
			}
		}
		return false;
	}

	void remove(int ID){
		buffer.remove(ID);
	}


private:
	int n;
	list<int> buffer;

};




#endif /* INCLUDES_CACHE_HPP_ */
