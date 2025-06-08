cmake_minimum_required(VERSION 3.10)

project(Laboratorio5)

set(OPENCV_DIR C:\Users\rushe\Documents\Universidad\S7\Graphics\CS-GRAFICA\Laboratorio5\build)
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})   