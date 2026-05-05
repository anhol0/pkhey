cmake_minimum_required(VERSION 3.20)
project(pkhey CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(PolkitQt6-1 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TSS2 REQUIRED tss2-esys tss2-rc tss2-mu)

add_executable(pkhey
    src/main.cpp
    src/Agent.cpp
    src/PinDialog.cpp
    src/TpmKeyStore.cpp
    src/PasskeyStore.cpp
)

target_link_libraries(pkhey )

target_link_libraries(pkhey
    PolkitQt6-1::Core 
    PolkitQt6-1::Gui 
    PolkitQt6-1::Agent
    ${TSS2_LIBRARIES}
)

target_include_directories(pkhey PRIVATE
    ${TSS2_INCLUDE_DIRS}
)
