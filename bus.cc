/* 
 * File:   bus.cc
 * Author: Abhinav
 * 
 * Created on October 26, 2012, 1:14 PM
 */

#include "cache.h"

class Bus {
    Cache *caches;
    int num;
public:
    Bus();
    ~Bus();
    
    void setCaches(Cache *caches){
        this->caches = caches;
    }
    
    void busRd(int id, ulong address){
        int i;
        for(i=0; i<num; i++)
            if(caches[i].getId() != id)
                caches[i].processMSIBusRd(address);
    }
    
    void busRdx(int id, ulong address){
        int i;
        for(i=0; i<num; i++)
            if(caches[i].getId() != id)
                caches[i].processMSIBusRdx(address);
    }
    
    void flush(int id, ulong address){
        //DoNothing
    }

private:

};
