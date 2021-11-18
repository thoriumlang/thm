#!/bin/sh

cmake -DCMAKE_BUILD_TYPE=Debug -Wdev -Wdeprecated -S . -B cmake-build-debug \
  && cmake  --build cmake-build-debug \
  && ctest --test-dir cmake-build-debug --output-on-failure