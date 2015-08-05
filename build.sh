#!/bin/bash

gcc -Ofast -ftree-vectorize -Wall -o net_speeder net_speeder.c -lpcap -lnet $1
