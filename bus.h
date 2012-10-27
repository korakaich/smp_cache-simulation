/* 
 * File:   bus.h
 * Author: Abhinav
 *
 * Created on October 26, 2012, 6:40 PM
 */
#ifndef BUS_H
#define	BUS_H

class Bus {
    Cache **caches;
    int num;
public:
    Bus(){
        
    }
    
    ~Bus();

    void setCaches(Cache **caches) {
        this->caches = caches;
    }

    void setNum(int num) {
        this->num = num;
    }

    void busRd(int, unsigned long);
    void busRdx(int, unsigned long);

    void flush(int id, unsigned long address) {
        //DoNothing
    }

private:

};
#endif	/* BUS_H */

