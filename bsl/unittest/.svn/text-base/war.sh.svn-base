make -j 16  > res 2>&1
cat res | grep "warning" | grep "^bsl_test" -v | sort | uniq  > 1
cat 1
