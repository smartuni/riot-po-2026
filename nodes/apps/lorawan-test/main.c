#include <stdio.h>
#include <stdlib.h>
#include "mate_lorawan.h"


int main (void){

    if(start_lorawan() < 0){
        printf("Error");
    }
}