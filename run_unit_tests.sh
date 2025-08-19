#! /usr/bin/env bash

# Run make run
make unittests

# Then build and run the tests
for test in build/tests/unit/*; do
    echo -e "\nSTARTING TEST $(basename $test):";
	./$test;
done
