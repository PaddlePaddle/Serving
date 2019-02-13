#!/bin/bash
for i in *.def
do
  bak=${i%%.def}.bak
  ./main $i 1>$bak.2
  diff $bak $bak.2 1>/dev/null 2>/dev/null
  if [ $? -ne  0 ]
      then
      echo "$i doesn't make the ccheck++ pass"
  else
      rm -rf $bak.2
  fi
done

