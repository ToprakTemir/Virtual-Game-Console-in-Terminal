#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <time.h>


#define char_car 'O'
#define char_obstacle '#'
#define char_empty ' '

#define road_width_inner 7
#define road_height 25
#define pixels_count (road_width_inner * road_height)
#define max_obstacle_density (1.0 / road_width_inner)
#define max_obstacles (road_width_inner * road_height * max_obstacle_density)
typedef struct {
    int car_x;
    int car_y; // constant

    int num_obstacles;

    // a pixel is either obstacle or empty, I only use it to track obstacles
    char pixels[road_width_inner * road_height]; // (x, y) for pixel i = (pixel[i] % width_inner, pixel[i] / width_inner)
} Road;

Road* road; // global so that we can free it in cleanup

Road* create_initial_road() {

    road = malloc(sizeof(Road));

    road->car_x = road_width_inner / 2;
    road->car_y = road_height - 1;

    road->num_obstacles = 0;

    // start with empty road, obstacles will spawn in the update function
    for (int i=0; i<pixels_count; i++)
        road->pixels[i] = char_empty;
    road->pixels[road->car_y * road_width_inner + road->car_x] = char_car;

    return road;
}

void print_initial_road(Road* road) {
    clear();
    for (int y = 0; y < road_height; y++) {
        printw("|");
        for (int x = 0; x < road_width_inner; x++) {
            printw("%c", road->pixels[x + y * road_width_inner]);
        }
        printw("|");
        printw("\n");
    }
    refresh();
}

void update_pixel(Road* road, const int x, const int y, const char c) {
    road->pixels[x + y * road_width_inner] = c;
    mvprintw(y, x+1, "%c", c); // +1 to skip the left border
    refresh();
}

#define right 1
#define left (-1)
#define neutral 0
int direction(char ch) {
    switch (ch) {
        case 'd': return right;
        case 'a': return left;
        default: return neutral;
    }
}

int lower_obstacles_and_check_collision(Road* road) {
    int collision = 0;
    for (int i=road_width_inner * road_height - 1; i >= 0; i--) {
        if (road->pixels[i] != char_obstacle)
            continue;

        int x = i % road_width_inner;
        int y = i / road_width_inner;

        int next_x = x;
        int next_y = y + 1;

        if (next_y == road->car_y && next_x == road->car_x) {
            collision = 1;
            // mvprintw(y, x+1, "X");
            update_pixel(road, next_x, next_y, 'X');
            refresh();
        }

        if (next_y == road_height) {
            update_pixel(road, x, y, char_empty);
            road->num_obstacles--;
        }
        else {
            update_pixel(road, x, y, char_empty);
            update_pixel(road, next_x, next_y, char_obstacle);
        }

    }
    return collision;
}

void spawn_obstacles_probabilistically(Road* road) {
    for (int i=0; i<road_width_inner; i++) {
        if (road->num_obstacles >= max_obstacles)
            return;

        int spawn = rand() % road_width_inner;
        while (road->pixels[spawn] == char_obstacle)
            spawn = rand() % road_width_inner;

        if (rand() < max_obstacle_density * RAND_MAX) {
            road->num_obstacles++;
            update_pixel(road, spawn, 0, char_obstacle);
        }

    }
}

int move_car_and_update_frame(Road* road, int direction) {
    int next_x = road->car_x + direction;
    if (next_x < 0 || next_x >= road_width_inner || road->pixels[road->car_y * road_width_inner + next_x] == char_obstacle)
        next_x = road->car_x;
    int next_y = road->car_y;


    if (next_x != road->car_x) {
        update_pixel(road, road->car_x, road->car_y, char_empty);
        update_pixel(road, next_x, next_y, char_car);
        road->car_x = next_x;
    }

    int collision = lower_obstacles_and_check_collision(road);
    spawn_obstacles_probabilistically(road);

    return collision;
}



// TODO
void cleanup() {
    clear();
    endwin();
    system("clear");
}

void handle_sigint() {
    cleanup();
    exit(0);
}

void handle_sigterm() {
    cleanup();
    exit(0);
}


int main() {
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigterm);

    initscr();             // Start ncurses mode
    cbreak();              // Disable line buffering
    keypad(stdscr, TRUE);  // Enable arrow keys
    noecho();              // Don't display typed characters
    nodelay(stdscr, TRUE); // make getch non-blocking
    curs_set(0);           // Hide the cursor

    road = create_initial_road();
    print_initial_road(road);

    const int fps = 5; // also affects the speed of the snake, BE CAREFUL
    const int frame_delay_ms = 1000 / fps; // delay between screen updates
    const int input_poll_ms = 20;

    int run = 1;
    int game_over = 0;
    struct timespec last_frame_time;
    clock_gettime(CLOCK_MONOTONIC, &last_frame_time);

    int last_valid_input = neutral;

    while (run) {

        char ch = getch();
        if (ch >= 'A' && ch <= 'Z')
            ch += 32; // make lowercase
        if (ch == 'q')
            run = 0;

        if (game_over)
            continue;

        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time_ms = (current_time.tv_sec - last_frame_time.tv_sec) * 1000
                             + (current_time.tv_nsec - last_frame_time.tv_nsec) / 1000000;

        if (direction(ch) != neutral) {
            last_valid_input = direction(ch);
        }

        if (elapsed_time_ms >= frame_delay_ms) {
            int collision = move_car_and_update_frame(road, last_valid_input);
            last_valid_input = neutral;
            game_over = collision;
            last_frame_time = current_time;
        }

        napms(input_poll_ms);
    }



    cleanup();
}