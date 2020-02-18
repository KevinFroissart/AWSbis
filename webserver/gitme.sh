#!/bin/sh
if [ $# != 1 ]; then
        echo "Nombre d'arguments invalides -> ./gitme --help"
elif [ "$1" = "--help" ]; then
        echo "./gitme \"commit message\""
else
        git add *;
        git commit -a -m $1;
        git push;
fi