#!/bin/bash
echo "Hello World!"
dir=finals2021

for file in $dir/*
do
	echo $file
	target=(${file//./ })
	target=${target[0]}
	echo $target
	echo "arm-linux-gnueabi-g++ -o" $target $file "-static -std=c++11"
	arm-linux-gnueabi-g++ -o $target $file -static -std=c++11
done
