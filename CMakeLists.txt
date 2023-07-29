cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
project (avp2_launcher)
set(CMAKE_C_STANDARD 17)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE HEADER_FILES ${INCLUDE_DIR}/*.h)

# Adding Raylib
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/burmaraider/raylib.git GIT_TAG master)
FetchContent_MakeAvailable(raylib)

# Directory Variables
set(RAYGUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/raygui-src)
set(RAYGUI_SRC ${RAYGUI_ROOT}/src)
include_directories(${RAYGUI_SRC})

set(PLATFORM "Windows")
set(ARCH "x64")

#check platform
if(WIN32)
  set(PLATFORM "Windows")
else()
  set(PLATFORM "Linux")
endif()

#check architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(ARCH "x64")
else()
  set(ARCH "x86")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/${PLATFORM}/${ARCH})
# set the output directories based on the build type
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/build/${PLATFORM}/${ARCH}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/build/${PLATFORM}/${ARCH}/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MinSizeRel ${CMAKE_CURRENT_SOURCE_DIR}/build/${PLATFORM}/${ARCH}/minsizerel)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RelWithDebInfo ${CMAKE_CURRENT_SOURCE_DIR}/build/${PLATFORM}/${ARCH}/RelWithDebInfo)

if(${PLATFORM} STREQUAL "Windows")
  if(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_MinSizeRel})
  endif()

  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
  endif()

  if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
  endif()

  if(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RelWithDebInfo})
  endif()
endif()

set(SOURCE_FILES 
${SOURCE_DIR}/Screens/advanced.c
${SOURCE_DIR}/Screens/splash.c
${SOURCE_DIR}/Screens/display.c
${SOURCE_DIR}/main.c
)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} )
target_link_libraries(${PROJECT_NAME} raylib
${CMAKE_CURRENT_SOURCE_DIR}/assets/MyApp.res)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})



if(MSVC)
  target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/assets/MyApp.rc)
  if(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  endif()
  if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  endif()
else()
    add_custom_command( TARGET ${PROJECT_NAME} PRE_BUILD COMMAND windres -i ${CMAKE_CURRENT_SOURCE_DIR}/assets/MyApp.rc -O coff ${CMAKE_CURRENT_SOURCE_DIR}/assets/MyApp.res)
    if(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
      set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE LINK_FLAGS "-mwindows")
    endif()
    if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
      set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE LINK_FLAGS "-mwindows")
    endif()
endif()

#add_custom_command(
#  TARGET ${PROJECT_NAME} POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${CMAKE_CURRENT_SOURCE_DIR}/assets
#    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)