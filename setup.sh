#!/bin/bash

# Checks the systems architecture and executes the corresponding setup.

architecture=$(arch)

if [[ $architecture == *"arm64"* ]]; then
  echo "Starting setup for arm64 host..."
  . ./setup_arm.sh
elif [[ $architecture == *"x86"* ]]; then
  echo "Starting setup for x86 host..."
  . ./setup_x86.sh
else
  echo "Host architecture not supported: $architecture"
fi


