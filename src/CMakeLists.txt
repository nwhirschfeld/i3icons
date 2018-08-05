cmake_minimum_required(VERSION 2.8)

project(i3icons)
add_executable(${PROJECT_NAME} "main.cpp" icons.config)

add_subdirectory(i3ipc++)

include_directories(${I3IPCpp_INCLUDE_DIRS})
link_directories(${I3IPCpp_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${I3IPCpp_LIBRARIES})


find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES})


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES icons.config DESTINATION etc/i3icons COMPONENT config)
