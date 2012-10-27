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
				caches[i]->processMESIBusRd(address);//change to moesi
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
				caches[i]->processMESIBusRdX(address);//change to moesi
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
