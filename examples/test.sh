#!/bin/bash
COMPILE=../../compile

#
# The purpose of this script is to verify if the compiler reports an error
# only when it is supposed to do so
#
# What this script does NOT do:
#  - check if programs compiling correctly have the correct execution behaviour
#

green()
{
	echo -e "\e[0;32m $@ \e[0m"
}

red()
{
	echo -e "\e[0;31m $@ \e[0m"
}

die()
{
    red $@
    exit 1
}

alert()
{
    red $@
}

test_failed()
{
    red TEST: $1 failed
}

test_passed()
{
    green TEST: $1 passed
}

test_correct_exo()
{
    for exo in *.u; do
    	rm -f a.out
	if ${COMPILE} $exo > /dev/null; then
	    if [ ! -f a.out ]; then
		alert $exo ': Missing executable'
	    fi	
	    test_passed $exo
	else
	    test_failed $exo
	fi
    done
}

test_incorrect_exo()
{
    for exo in *.u; do
	if ${COMPILE} $exo > /dev/null 2>&1; then
	    test_failed $exo
	else
	    test_passed $exo
	fi
    done
}

cd $(dirname $0)
if [ ! -f ../uuc ]; then
	echo "Erreur compilateur introuvable; Pensez à faire \"make\" (à la racine)"
	exit 1
fi

for i in *; do
    if [ -d $i  ]; then
	pushd $i > /dev/null
	if [ -f correct.txt ]; then
	    test_correct_exo
	elif [ -f incorrect.txt ]; then
	    test_incorrect_exo
	else
	    die $i : "Unexpected error"
	fi
	popd > /dev/null
    fi
done
