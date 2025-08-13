#!/bin/bash

if [ -z "$1" ]; then
  echo "Error: No engine port numbers provided."
  echo "Usage: $0 <port1> <port2> <port3> ..."
  exit 1
fi

DRIVER_EXECUTABLE="./build/driver"

$DRIVER_EXECUTABLE "$@"

