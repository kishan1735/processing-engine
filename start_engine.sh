#!/bin/bash

if [ -z "$1" ]; then
  echo "Error: No port number provided."
  echo "Usage: $0 <port_number>"
  exit 1
fi

ENGINE_EXECUTABLE="./build/engine"

$ENGINE_EXECUTABLE $1 &