#----------------------------------------------------------------------
#----------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5.1)

project(grpc-perf C CXX)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  cmake_policy(SET CMP0091 NEW)
  add_definitions(-D_WIN32_WINNT=0x600 -bigobj)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

#----------------------------------------------------------------------
# Sockets Server
#----------------------------------------------------------------------
add_executable(sockets_server
  server.cc
  )
target_link_libraries(sockets_server
   ${_REFLECTION}
   )

#----------------------------------------------------------------------
# Sockets Client
#----------------------------------------------------------------------
add_executable(sockets_client
  client.cc
  )
target_link_libraries(sockets_client
  ${_REFLECTION}
  )
