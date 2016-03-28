#!/bin/sh

for toolchain in clang atom
do
  for action in configure make
  do
    qibuild $action -c $toolchain --build-type RelWithDebInfo
  done
done

