/* 
 * File:   bus.cc
 * Author: Abhinav
 * 
 * Created on October 26, 2012, 1:14 PM
 */

#include "cache.h"
#include <iostream>
using namespace std;
   
Bus::Bus(int protocol){this->protocol=protocol;}
Bus::~Bus(){}
void Bus::setCaches(Cache **caches, int num)
{
    this->caches = caches;
	numOfCaches=num;
}
   
void Bus::busRd(int id, ulong address)
{
    for(int i=0;i<numOfCaches;i++)
	{
		if(i!=id)
		{
			//call cache function
			if(protocol==0)
				caches[i]->processMSIBusRd(address);
			else if(protocol==1)
				caches[i]->processMESIBusRd(address);
			else
				caches[i]->processMOESIBusRd(address);//change to moesi
		}    
	}
}   

void Bus::busRdX(int id, ulong address)
{   
	for(int i=0;i<numOfCaches;i++)
	{
		if(i!=id)
		{
			if(protocol==0)
				caches[i]->processMSIBusRdX(address);
			else if(protocol==1)
				caches[i]->processMESIBusRdX(address);
			else
				caches[i]->processMOESIBusRdX(address);//change to moesi
		}    
	}
}

void Bus::busUpgr(int id, ulong address)
{
	for(int i=0;i<numOfCaches;i++)
	{
		if(i!=id)
		{
			//call cache function
			if(protocol==1)
				caches[i]->processMESIBusUpgr(address);
			else
				caches[i]->processMOESIBusUpgr(address);
		}    
	}
}

bool Bus::isCached(int id, ulong addr)
{
	for(int i=0;i<numOfCaches;i++)
	{
		if(i!=id)
		{
			if(caches[i]->findLine(addr) !=NULL)
				return true;
		}
	}
	return false;
}

bool Bus::isCachedOwner(int id, ulong addr) {
    for (int i = 0; i < numOfCaches; i++) {
        if (i != id) {
            cacheLine *temp = caches[i]->findLine(addr);
            if (temp != NULL && !temp->isInvalid())
                    return true;                          
        }
    }
    return false;
}

bool Bus::isCachedDirty(int id, ulong addr) {
    for (int i = 0; i < numOfCaches; i++) {
        if (i != id) {
            cacheLine *temp = caches[i]->findLine(addr);
            if (temp != NULL && (temp->isOwner() || temp->isModified() || temp-> isExclusive()))
                    return true;                          
        }
    }
    return false;
}
