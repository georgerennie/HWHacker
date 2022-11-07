cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(hw_hacker_attacker)
pico_sdk_init()

set(CMAKE_CXX_STANDARD 20)

set(target hw_hacker_attacker)
set(src ${CMAKE_CURRENT_LIST_DIR}/src)
set(libs ${CMAKE_CURRENT_LIST_DIR}/libs)

set(target_fw ${CMAKE_CURRENT_LIST_DIR}/target_fw)
set(target_fw_build ${target_fw}/build)
set(target_fw_h ${target_fw_build}/target_fw.h)

# Useful for reloading firmware when developing
if (DEV_PICO_BOOTSEL_RESET)
	add_subdirectory(${libs}/pico_bootsel_reset)
	target_link_libraries(${target} bootsel_reset)
endif()

# Useful for development on linux
if (DEV_PICO_SDK_COMPILE_COMMANDS)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	set(GCC_INCLUDE /usr/${PICO_GCC_TRIPLE}/include)
	include_directories(${GCC_INCLUDE})
	include_directories(${GCC_INCLUDE}/c++/${CMAKE_CXX_COMPILER_VERSION})
	include_directories(${GCC_INCLUDE}/c++/${CMAKE_CXX_COMPILER_VERSION}/${PICO_GCC_TRIPLE})
endif()

add_executable(${target}
	${src}/main.cpp

	${src}/rp2040/hardware.h
	${src}/rp2040/hardware.cpp

	${src}/target_interfaces/pins.h
	${src}/target_interfaces/target_controller.cpp
	${src}/target_interfaces/target_controller.h
	${src}/target_interfaces/spi_prog.cpp
	${src}/target_interfaces/spi_prog.h
	${src}/target_interfaces/clk_controller.cpp
	${src}/target_interfaces/clk_controller.h
	${src}/target_interfaces/serial.cpp
	${src}/target_interfaces/serial.h
)

target_include_directories(${target} PRIVATE ${libs}/PIO_Builder)

# Set boot stage 2 param to allow overclocking
target_compile_definitions(bs2_default PRIVATE PICO_FLASH_SPI_CLKDIV=4)

# Build target firmware
add_custom_target(
	target_fw
	COMMAND make all
	WORKING_DIRECTORY ${target_fw}
)
add_dependencies(${target} target_fw)
target_include_directories(${target} PRIVATE ${target_fw_build})

pico_enable_stdio_usb(${target} 0)
pico_enable_stdio_uart(${target} 1)

target_link_libraries(${target}
	pico_stdlib
	hardware_spi
	hardware_pio
	hardware_dma
	pico_multicore
)

pico_add_extra_outputs(${target})
