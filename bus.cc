/* 
 * File:   bus.cc
 * Author: Abhinav
 * 
 * Created on October 26, 2012, 6:40 PM
 */

#include "cache.h"

void Bus::busRd(int id, unsigned long address) {
    int i;
    for (i = 0; i < num; i++)
        if (caches[i]->getId() != id)
            caches[i]->processMSIBusRd(address);
}

void Bus::busRdx(int id, unsigned long address) {
    int i;
    for (i = 0; i < num; i++)
        if (caches[i]->getId() != id)
            caches[i]->processMSIBusRdx(address);
}
