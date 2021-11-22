#!/bin/sh

cmake --build cmake-build-debug \
  && ctest --test-dir cmake-build-debug --output-on-failure