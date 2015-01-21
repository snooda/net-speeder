#!/bin/bash

gcc -o net_speeder net_speeder.c -lpcap -lnet $1
