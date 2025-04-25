cmake_minimum_required(VERSION 3.16)
project(SoundOverFlow)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)

file(GLOB SOURCES mus/*.cpp mus/*.h)

add_executable(RayLibTemplate ${SOURCES})
# Setting parameters for raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or 

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/SQLiteCpp)
add_subdirectory(libs/raylib-5.5 ./libs/raylib)
target_link_libraries(RayLibTemplate PRIVATE raylib)
target_link_libraries(RayLibTemplate PRIVATE propsys)
target_link_libraries(RayLibTemplate PRIVATE shlwapi)
target_link_libraries(RayLibTemplate PRIVATE comctl32)
# add_custom_command(TARGET RayLibTemplate POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/dvd/dvd.png" $<TARGET_FILE_DIR:RayLibTemplate >/dvd.png
# )add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp)

target_link_libraries(RayLibTemplate PRIVATE SQLiteCpp)
target_link_libraries(RayLibTemplate PRIVATE sqlite3)
target_link_libraries(RayLibTemplate PRIVATE pthread)
# target_link_libraries(RayLibTemplate PRIVATE dl)