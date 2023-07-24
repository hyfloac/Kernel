#!/bin/zsh

docker cp boot0 $1:/build
docker cp boot1 $1:/build
docker cp boot2 $1:/build
docker cp crt $1:/build
docker cp kernel $1:/build

docker cp $1:/build/bootloader.img .
