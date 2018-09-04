cmake_minimum_required(VERSION 3.8.2)
project(sikradio)


ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

FIND_PACKAGE( Boost 1.6.1 COMPONENTS program_options REQUIRED )
find_package (Threads)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -O2")

# http://stackoverflow.com/questions/10555706/
macro (add_executable _name)
    # invoke built-in add_executable
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} LINK_PUBLIC ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
    endif()
endmacro()

add_executable(sikradio-sender
        sender/src/sikradio-sender.cpp
        sender/src/CtrlController.cpp
        sender/include/CtrlController.h
        messages/messages.cpp
        messages/messages.h
        utils/err.cpp
        utils/err.h
        sender/src/RetransmissionController.cpp
        sender/include/RetransmissionController.h
        sender/include/Fifo.h
        sender/include/DataController.h sender/src/DataController.cpp sender/src/Fifo.cpp)

add_executable(sikradio-receiver
        receiver/src/sikradio-receiver.cpp
        utils/err.cpp utils/err.h
        receiver/src/MenuController.cpp
        receiver/include/MenuController.h
        receiver/src/StationController.cpp
        receiver/include/StationController.h
        receiver/src/DataController.cpp
        receiver/include/DataController.h
        messages/messages.cpp
        messages/messages.h
        receiver/include/Sender.h
        receiver/src/CtrlController.cpp
        receiver/include/CtrlController.h
        receiver/src/Buffer.cpp
        receiver/include/Buffer.h
        receiver/src/RetransmissionController.cpp
        receiver/include/RetransmissionController.h)