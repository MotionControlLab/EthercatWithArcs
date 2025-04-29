if [ ! -d build ]; then
    mkdir build
fi
cd build

cmake ..
if [ $? -ne 0 ]; then
    echo "CMake configuration failed"
    exit 1
fi

make
if [ $? -ne 0 ]; then
    echo "Make failed"
    exit 1
fi

sudo ./src/test eth0
