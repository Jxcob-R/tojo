#! /usr/bin/env bash

# Clear built files and re-build
make
make unittests

# Then build and run the tests
for test in build/tests/unit/*; do
    echo -e "\nSTARTING TEST $(basename $test):";
	./$test;
done
