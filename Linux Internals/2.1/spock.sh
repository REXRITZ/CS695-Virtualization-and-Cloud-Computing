#!/bin/bash

make clean

make

gcc spock_user.c -o spock_user

insmod spock_driver.ko

./spock_user

rmmod spock_driver