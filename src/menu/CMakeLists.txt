file(GLOB_RECURSE
    MENU_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(menu
    SHARED
    ${MENU_FILES}
)

target_link_libraries(menu PRIVATE SDL3::SDL3)
target_compile_definitions(menu PRIVATE BUILDING_PLUGIN_DLL)

set_target_properties(menu PROPERTIES
    PREFIX ""
    OUTPUT_NAME "menu"
)
