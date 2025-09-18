cmake_minimum_required(VERSION 3.10)

find_package(Protobuf REQUIRED)
set(PROTO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/task.proto)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC})

add_library(proto_messages STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(proto_messages PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(proto_messages PUBLIC ${Protobuf_LIBRARIES})
