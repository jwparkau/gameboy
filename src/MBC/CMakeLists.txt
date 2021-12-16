target_sources(gameboy PUBLIC
	nombc.cpp
	mbc1.cpp
	mbc2.cpp
	mbc3.cpp
)

target_include_directories(gameboy PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
