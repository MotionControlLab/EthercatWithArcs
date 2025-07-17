#!/bin/bash

# エラーでスクリプトを終了
set -e

cd ./ARCS6/robot/Soem/BaseCtrl

# cmake
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..

# ビルドと実行
# 引数が "offline" の場合はオフラインモードでビルド
if [ $1 == "offline" ]; then
    make -j ARCS_offline
    sudo ./ARCS_offline
else 
    make -j ARCS
    sudo ./ARCS
fi
