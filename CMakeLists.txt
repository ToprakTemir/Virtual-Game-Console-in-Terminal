cmake_minimum_required(VERSION 3.29)
project(VirtualGameConsole C)

set(CMAKE_C_STANDARD 23)

add_executable(VirtualGameConsole
        src/game_snake.c
        src/main-screen.c
        src/game_blackjack.c
        src/game_racing.c
)
