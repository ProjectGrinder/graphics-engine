add_subdirectory(menu)

file(GLOB
    SRC_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
)

target_link_libraries(
    ${PROJECT_NAME}
    SDL3::SDL3 SDL3_shadercross::SDL3_shadercross
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:graphics-lab-engine>
        $<TARGET_FILE_DIR:graphics-lab-engine>
    COMMAND_EXPAND_LISTS
)
