file(GLOB_RECURSE
    SRC_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
)

target_link_libraries(${PROJECT_NAME} SDL3::SDL3 SDL3_shadercross::SDL3_shadercross)
