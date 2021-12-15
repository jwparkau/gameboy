target_sources(gameboy PUBLIC
	channel.cpp
	envelope.cpp
	length.cpp
	sweep.cpp
)

target_include_directories(gameboy PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
