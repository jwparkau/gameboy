cmake_minimum_required(VERSION 3.21)

project(
	GameBoy_Emulator
	VERSION 0.1
	LANGUAGES CXX
)

add_executable(gameboy "")

add_subdirectory(src)
