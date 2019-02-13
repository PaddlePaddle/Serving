cd ../../
rm -r lib2/bsl -f
cvs co lib2/bsl

rm -r lib2-64/bsl -f
cvs co lib2-64/bsl

rm -r lib2-64/ullib -f
cvs co lib2-64/ullib

rm -r lib2/ullib -f
cvs co lib2/ullib

rm -r third/pcre -f
cvs co third/pcre

rm -r third-64/pcre -f
cvs co third-64/pcre

rm -r public/spreg -f
cvs co public/spreg
make -C public/spreg

cd public/configure
make 
