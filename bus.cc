/* 
 * File:   bus.cc
 * Author: Abhinav
 * 
 * Created on October 26, 2012, 1:14 PM
 */

#include "bus.h"
#include <iostream>
using namespace std;
   
Bus::Bus(){}
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
		}    
	}
}   

void Bus::busRdX(int id, ulong address)
{   
	for(int i=0;i<numOfCaches;i++)
	{
		if(i!=id)
		{
			//call cache function
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
		}    
	}
}

void Bus::flush(int id, ulong address)
{
	for(int i=0;i<numOfCaches;i++)
	{
		if(i!=id)
		{
			//call cache function
		}    
	}
}


