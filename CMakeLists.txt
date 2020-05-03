cmake_minimum_required(VERSION 3.10)

project(cli-fm VERSION 0.1)

add_executable(cli-fm
	src/main.c
	src/cdpath.c
	src/file_manipulation.c
	src/vector.c
	)

install(TARGETS cli-fm DESTINATION bin)
