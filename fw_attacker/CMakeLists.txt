cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(hw_hacker_attacker)
pico_sdk_init()

set(fw_target hw_hacker_attacker)
set(src ${CMAKE_CURRENT_LIST_DIR}/src)
set(libs ${CMAKE_CURRENT_LIST_DIR}/libs)

# Useful for reloading firmware when developing
if (DEV_PICO_BOOTSEL_RESET)
	add_subdirectory(${libs}/pico_bootsel_reset)
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

add_executable(${fw_target}
	${src}/main.cpp

	${src}/target_interfaces/io_arbiter.h
	${src}/target_interfaces/spi_prog.cpp
	${src}/target_interfaces/spi_prog.h
	${src}/target_interfaces/clk_controller.cpp
	${src}/target_interfaces/clk_controller.h
)

target_include_directories(${fw_target} PRIVATE ${libs}/PIO_Builder)

pico_enable_stdio_usb(${fw_target} 1)
pico_enable_stdio_uart(${fw_target} 0)

target_link_libraries(${fw_target}
	pico_stdlib
	hardware_spi
	hardware_pio
	hardware_dma
	bootsel_reset
)

pico_add_extra_outputs(${fw_target})
