cmake_minimum_required(VERSION 3.8.2)
project(sikradio)


ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

FIND_PACKAGE( Boost 1.6.1 COMPONENTS program_options log REQUIRED )
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
        sender/sikradio-sender.cpp
        sender/controllers/CtrlController.cpp
        sender/controllers/CtrlController.h
        messages/messages.cpp
        messages/messages.h
        utils/err.cpp
        utils/err.h
        sender/controllers/RetransmissionController.cpp
        sender/controllers/RetransmissionController.h
        sender/Fifo.h
        sender/controllers/DataController.h sender/controllers/DataController.cpp sender/Fifo.cpp)

add_executable(sikradio-receiver
        receiver/sikradio-receiver.cpp
        utils/err.cpp utils/err.h
        receiver/controllers/MenuController.cpp
        receiver/controllers/MenuController.h
        receiver/controllers/StationController.cpp
        receiver/controllers/StationController.h
        receiver/controllers/DataController.cpp
        receiver/controllers/DataController.h
        messages/messages.cpp
        messages/messages.h
        receiver/Sender.h
        receiver/controllers/CtrlController.cpp
        receiver/controllers/CtrlController.h
        receiver/Buffer.cpp
        receiver/Buffer.h
        receiver/controllers/RetransmissionController.cpp
        receiver/controllers/RetransmissionController.h )