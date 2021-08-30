cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(hw_hacker_attacker)
pico_sdk_init()

# Useful for reloading firmware when developing
if (DEV_PICO_BOOTSEL_RESET)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/pico_bootsel_reset)
endif()

# Useful for development on linux
if (DEV_PICO_SDK_COMPILE_COMMANDS)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	set(GCC_INCLUDE /usr/${PICO_GCC_TRIPLE}/include)
	include_directories(${GCC_INCLUDE})
	include_directories(${GCC_INCLUDE}/c++/${CMAKE_CXX_COMPILER_VERSION})
	include_directories(
		${GCC_INCLUDE}/c++/${CMAKE_CXX_COMPILER_VERSION}/${PICO_GCC_TRIPLE}
	)
endif()

set(fw_target hw_hacker_attacker)
set(src ${CMAKE_CURRENT_LIST_DIR}/src)

add_executable(${fw_target}
	${src}/main.cpp

	${src}/target_interfaces/spi_prog/spi_prog.cpp
	${src}/target_interfaces/spi_prog/spi_prog.h
)

pico_enable_stdio_usb(${fw_target} 1)
pico_enable_stdio_uart(${fw_target} 0)

target_link_libraries(${fw_target} pico_stdlib hardware_spi bootsel_reset)

pico_add_extra_outputs(${fw_target})
