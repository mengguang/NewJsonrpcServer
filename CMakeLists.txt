cmake_minimum_required(VERSION 3.14)
project(NewJsonrpcServer C)

set(CMAKE_C_STANDARD 11)

add_executable(NewJsonrpcServer main.c cJSON.c cJSON.h newjsonrpc.h newjsonrpc.c)
target_link_libraries(NewJsonrpcServer m)