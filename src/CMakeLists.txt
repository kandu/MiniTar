add_library(minitar
    portable_endian.h
    minitar.cpp
    )

target_include_directories(minitar PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(minitar PUBLIC cxx_std_17)
target_compile_options(minitar PUBLIC -std=c++17)
target_link_libraries(minitar PUBLIC)

install(TARGETS minitar
    EXPORT minitarTargets
    DESTINATION lib
    INCLUDES DESTINATION include
    )
install(FILES minitar.hpp DESTINATION include)

