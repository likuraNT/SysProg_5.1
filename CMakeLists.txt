cmake_minimum_required(VERSION 3.17.0)

project(utility VERSION 1.0)

set(DIR_INCLUDE include)

add_executable(${PROJECT_NAME} src/main.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${DIR_INCLUDE})