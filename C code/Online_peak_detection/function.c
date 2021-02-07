#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <string.h>
#include<malloc.h>
#include <math.h>
#include <unistd.h>

int a[] = [1,2,3,4,5,6,7,8];

bool isbigger = False;
for (int i =0;i<8;i++){
        if(a[i]>9){
            isbigger = True ;
        }
}
