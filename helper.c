#include <stdio.h>

void print_array(unsigned char* array, int length){
    printf("\n");
    /*
    for(int i=0;i<length;i++){
        printf(" %02X",array[i]);

        if((i+1)%16==0){
            printf("\n");
        }
    }*/
    printf("\n");
}

void copy_array(unsigned char* dest, unsigned char* src,int length){
    for(int i=0;i<length;i++){
        dest[i]=src[i];
    }
}

