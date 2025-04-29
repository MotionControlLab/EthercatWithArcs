cmake_minimum_required(VERSION 3.0)

add_executable(test 
    main.cc
    ethercat_bus.cc
)

target_link_libraries(test soem)
