cmake_minimum_required(VERSION 3.20)
project(Win32TitanicMDI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AppA WIN32
    AppA/main.cpp
    AppA/MDIFrame.cpp
    AppA/OA1Window.cpp
    AppA/OA2Window.cpp
    AppA/TitanicData.cpp
    AppA/ClipboardHelper.cpp
    AppA/PipeLauncher.cpp
    AppA/DDEServer.cpp
    AppA/main.cpp AppA/MDIFrame.cpp AppA/OA1Window.cpp AppA/OA2Window.cpp
    AppA/TitanicData.cpp AppA/ClipboardHelper.cpp AppA/PipeLauncher.cpp
    AppA/DDEServer.cpp AppA/DDEClient.cpp
 "AppA/DDEClient.cpp")
target_include_directories(AppA PRIVATE AppA)
target_link_libraries(AppA user32 gdi32)
target_compile_definitions(AppA PRIVATE UNICODE _UNICODE)



add_executable(AppB WIN32
    AppB/main.cpp
 "AppA/DDEClient.cpp")
target_include_directories(AppB PRIVATE AppB)
target_link_libraries(AppB user32 gdi32)
target_compile_definitions(AppB PRIVATE UNICODE _UNICODE)

add_dependencies(AppA AppB)

add_custom_command(TARGET AppA POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AppB>
        $<TARGET_FILE_DIR:AppA>/AppB.exe
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:AppA>/data
    COMMENT "Copying AppB.exe and data/ next to AppA.exe"
)