#!/bin/bash

stty -icanon -echo
./test1
stty icanon echo
echo

