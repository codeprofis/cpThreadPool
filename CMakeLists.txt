cmake_minimum_required(VERSION 3.14)
project(CpThreadPool)

set(CMAKE_CXX_STANDARD 17)

add_library(CpThreadPool CpThreadPool/CpThreadPool.cpp CpThreadPool/CpThreadPool.h CpThreadPool/CpPrioThreadPool.cpp CpThreadPool/CpPrioThreadPool.h)
TARGET_LINK_LIBRARIES(CpThreadPool threads)