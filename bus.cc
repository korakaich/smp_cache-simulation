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
        
    }
    
    void busRdx(int id, ulong address){
        
    }
    
    void flush(int id, ulong address){
        
    }

private:

};
