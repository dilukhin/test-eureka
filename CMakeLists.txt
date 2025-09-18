cmake_minimum_required(VERSION 3.10)
project(qt_task_agent_controller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network Sql)
find_package(Protobuf REQUIRED)

add_subdirectory(proto)

add_library(common STATIC
    common/ProtoChannel.cpp
    common/ProtoChannel.h
)
target_link_libraries(common PUBLIC Qt5::Core Qt5::Network proto_messages ${Protobuf_LIBRARIES})
target_include_directories(common PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# Agent
add_executable(agent
    agent/agent_main.cpp
    agent/AgentWindow.cpp
    agent/AgentWindow.h
    agent/AgentClient.cpp
    agent/AgentClient.h
    agent/SumTaskWorker.cpp
    agent/SumTaskWorker.h
)
target_link_libraries(agent
    PRIVATE Qt5::Core Qt5::Widgets Qt5::Network proto_messages common ${Protobuf_LIBRARIES}
)

# Controller
add_executable(controller
    controller/controller_main.cpp
    controller/ControllerWindow.cpp
    controller/ControllerWindow.h
    controller/ControllerServer.cpp
    controller/ControllerServer.h
    controller/ControllerDb.cpp
    controller/ControllerDb.h
)
target_link_libraries(controller
    PRIVATE Qt5::Core Qt5::Widgets Qt5::Network Qt5::Sql proto_messages common ${Protobuf_LIBRARIES}
)

set_target_properties(agent controller PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "$ORIGIN"
)
