cmake_minimum_required(VERSION 3.16)

project(DiplomaServer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_executable(DiplomaServer
  main.cpp
)
target_link_libraries(DiplomaServer Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(DiplomaServer Qt${QT_VERSION_MAJOR}::Network)
target_include_directories(DiplomaServer PRIVATE ${CMAKE_SOURCE_DIR}/Files)

include(GNUInstallDirs)
install(TARGETS DiplomaServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(PATH_VARIABLE "${CMAKE_SOURCE_DIR}/Files")
file(COPY ${PATH_VARIABLE} DESTINATION ${CMAKE_BINARY_DIR})
