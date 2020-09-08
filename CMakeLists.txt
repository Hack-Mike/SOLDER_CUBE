# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "SCB_0.0.0")

include(/Users/michelemocellin/esp/esp-idf-v4.0.1/tools/cmake/project.cmake)
project(SOLDER_CUBE C CXX ASM)

target_add_binary_data(SOLDER_CUBE.elf "Certificate/MQTT/mqtt_eclipse_org.pem" TEXT)




