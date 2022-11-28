set(EXEC_NAME "knock")

set(Source_Files "knock.cpp")
source_group("Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Source_Files}")

if (MSVC)
  set_property(TARGET "${EXEC_NAME}" APPEND_STRING PROPERTY LINK_FLAGS " /NODEFAULTLIB:MSVCRTD")
endif (MSVC)

target_compile_definitions(
        "${EXEC_NAME}"
        PRIVATE
        KNOCK_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${PROJECT_VERSION_TWEAK}"
)

find_package(CURL CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(libgourou CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(updfparser CONFIG REQUIRED)
target_link_libraries(
        "${EXEC_NAME}"
        PRIVATE
        "${PROJECT_NAME}_compiler_flags"
        "gourou"
        "gourou_utils"
        "gourou_include"
        "gourou_compiler_flags"
        "CURL::libcurl"
)

set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        CXX
)

# install rules
include(GNUInstallDirs)

install(TARGETS "${EXEC_NAME}"
        # TYPE "RUNTIME"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        COMPONENT           "${PROJECT_NAME}")
