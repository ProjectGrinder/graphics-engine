file(GLOB
    SRC_FILES
    "./*.cpp"
    "./*.h"
    "./**/*.cpp"
    "./**/*.h"
)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
)

target_link_libraries(${PROJECT_NAME} SDL3::SDL3 SDL3_shadercross::SDL3_shadercross)
