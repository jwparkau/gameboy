target_compile_options(gameboy PUBLIC -Wall -Wextra -O2)

set_target_properties(gameboy
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

find_package(SDL2 REQUIRED)

target_sources(gameboy PUBLIC
	cartridge.cpp
	cpu.cpp
	gameboy.cpp
	main.cpp
	memory.cpp
	platform.cpp
	timer.cpp
	types.cpp
	ppu.cpp
)

target_include_directories(gameboy PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gameboy SDL2::SDL2)

add_subdirectory(MBC)
