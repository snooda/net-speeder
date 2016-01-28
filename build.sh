#!/bin/bash

gcc -O3 -ffast-math -ftree-vectorize -o netspeeder netspeeder.c -lpcap -lnet $1
chmod 777 netspeeder
cp netspeeder /bin/
