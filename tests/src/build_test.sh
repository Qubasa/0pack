#!/bin/sh

gcc -fpie -pie test.c -o ../bins/pie_test.elf
gcc -fPIC -no-pie test.c -o ../bins/no_pie_test.elf
