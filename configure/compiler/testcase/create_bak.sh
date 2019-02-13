#!/bin/bash
if [ $# -eq 1 ] 
then
    i=$1
    bak=${i%%.def}.bak
    ./main $i 1>$bak
else
    for i in *.def
      do
      bak=${i%%.def}.bak
      ./main $i 1>$bak
    done
fi

