#!/bin/bash

# エラーでスクリプトを終了
set -e

cd ./ARCS6/robot/Soem/BaseCtrl

# ビルド
mkdir -p build && cd build
cmake ..
make

# 実行
sudo ./ARCS
