#!/bin/bash

# Checks the systems architecture and executes the corresponding setup.

architecture=$(arch)

if [[ $architecture == *"aarm64"* ]]; then
  echo "Starting setup for arm64 host..."
elif [[ $architecture == *"x86"* ]]; then
  echo "Starting setup for x86 host..."
else
  echo "Host architecture not supported: $architecture"
fi


