#!/bin/bash

if [ "$#" -ge 1 ]
then
    DEST=$1
    if [ -d "$DEST" ]
    then
        echo "Installing in $DEST"
    else
        echo "$DEST Does not exist as a directory"
        exit 1
    fi
else
    DEST="$HOME/.local/bin"
    echo "Using default install at $DEST"
fi

echo -n "Do you want to continue? (Y/n): "
read -r answer
if [[ ! "$answer" =~ ^[yY]$ ]]
then
    echo "Cancelled"
    exit 0
fi

make && make final
if [ ! "$?" ]
then
    echo "Failed to build project"
    exit 2
fi

mv tojo "$DEST" 
if [ ! "$?" ]
then
    echo "Failed to install, may require sudo"
    make
    exit 3
fi

make
