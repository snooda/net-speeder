#!/bin/bash

gcc -O2 -o net_speeder net_speeder.c -lpcap -lnet $1
