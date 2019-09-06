cmake_minimum_required(VERSION 3.10)
project(CpThreadPool)

set(CMAKE_CXX_STANDARD 17)

EXECUTE_PROCESS( COMMAND gcc -dumpversion GCC_VERSION )
message("${GCC_VERSION}")

# Set variables
include(${CMAKE_SOURCE_DIR}/cmake/SetEnv.cmake)

add_subdirectory(examples)

# Library sources
add_subdirectory(${LIBRARY_FOLDER})

#add_library(CpThreadPool CpThreadPool/CpThreadPool/CpThreadPool.cpp CpThreadPool/CpThreadPool/CpThreadPool.h CpThreadPool/CpThreadPool/CpPrioThreadPool.cpp CpThreadPool/CpThreadPool/CpPrioThreadPool.h)
#TARGET_LINK_LIBRARIES(CpThreadPool threads)

#install(TARGETS bar
        # In order to export target, uncomment next line
        #   EXPORT ${PROJECT_EXPORT}
 #       RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)

# Install targets
include(${CMAKE_SOURCE_DIR}/cmake/InstallConfig.cmake)

