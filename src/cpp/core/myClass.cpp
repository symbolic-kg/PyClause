#include "myClass.h"
#include <string>


void myClass::addOne() {
    number = number + 1;
}

int myClass::getNumber() {
    return number;
}

long myClass::addRange(long start, long end){
    long sum = 0;
    for (long i=start; i<=end; ++i){
        sum += i;
    }
    return sum;
}

