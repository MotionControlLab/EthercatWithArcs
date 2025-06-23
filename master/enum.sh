#!/bin/bash

# イーサポート列挙

# https://qiita.com/p_cub/items/43c5c44886443087de09

for DEV in `find /sys/devices -name net | grep -v virtual`; do ls $DEV/; done
