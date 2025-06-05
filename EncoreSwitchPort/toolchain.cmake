set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(DEVKITPRO /opt/devkitpro)
set(DEVKITARM ${DEVKITPRO}/devkitA64)

set(CMAKE_C_COMPILER "${DEVKITARM}/bin/aarch64-none-elf-gcc")
set(CMAKE_CXX_COMPILER "${DEVKITARM}/bin/aarch64-none-elf-g++")
set(CMAKE_AR "${DEVKITARM}/bin/aarch64-none-elf-ar" CACHE STRING "Archiver")
set(CMAKE_RANLIB "${DEVKITARM}/bin/aarch64-none-elf-ranlib" CACHE STRING "Ranlib")

set(CMAKE_C_FLAGS "-g -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE" CACHE STRING "C flags")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -fno-rtti" CACHE STRING "C++ flags")
set(CMAKE_EXE_LINKER_FLAGS "-L${DEVKITPRO}/libnx/lib -L${DEVKITPRO}/portlibs/switch/lib -lnx" CACHE STRING "Linker flags")

set(CMAKE_FIND_ROOT_PATH "${DEVKITPRO}/libnx" "${DEVKITPRO}/portlibs/switch")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)