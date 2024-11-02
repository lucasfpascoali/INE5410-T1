cmake_minimum_required(VERSION 3.5)

project(rally_marciano LANGUAGES C)

add_executable(rally_marciano rally_marciano.c)

target_link_libraries(rally_marciano PRIVATE pthread)
