#!/usr/bin/env bash


function main() {
   cat Serving/doc/doc_test_list | xargs python Serving/tools/doc_tester_reader.py Serving/doc/ 

}


main $@
