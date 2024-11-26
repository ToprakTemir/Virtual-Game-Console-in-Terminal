#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <time.h>

#define width 20
#define height 25

#define char_head 'O'
#define char_tail '#'
#define char_bait 'X'
#define char_empty '.'


typedef struct {
    char* cells; // 1D array of cells implemented treated as a 2D array

    /*  1D circular array, having the max size snake can have. It is actually a circular array, keeping the past positions
     *  of the snake head. To get the full position of the snake, start from the head and trace back the array snake_length times
     */
    int* snake;
    int snake_head_idx;
    int snake_length;
    int snake_direction; // 0: up, 1: right, 2: down, 3: left
} Board;


Board* board; // global so that we can free it in cleanup


/* top left corner: (0, 0)
* bottom right corner: (width-1, height-1) */
int indexOf(int x, int y) {
    return x + y * width;
}

Board* create_initial_board() {
    // set seed as current time for rand
    srand(time(NULL));

    board = malloc(sizeof(Board));

    if (board == NULL) {
        printf("Failed to allocate memory for the board\n");
        return NULL;
    }

    board->cells = (char*) malloc(sizeof(char*) * width * height);
    board->snake = (int*) malloc(sizeof(int) * width * height);
    board->snake_length = 2;
    board->snake_head_idx = 1;

    // initial field of empty cells
    for (int i = 0; i < width * height; i++) {
        board->cells[i] = char_empty;
    }

    // put snake in the middle
    int snake_pos = width * height / 2;

    board->cells[snake_pos] = char_head;
    board->snake[board->snake_head_idx] = snake_pos;

    // decide tail position, currently always placed on the left
    int tail_pos = snake_pos - 1;
    board->cells[tail_pos] = char_tail;
    board->snake[board->snake_head_idx - 1] = tail_pos;

    // decide initial direction, currently always going to the right
    int snake_direction = 1;
    board->snake_direction = snake_direction;

    // put bait
    int bait_pos = rand() % (width * height);

    // ensure bait is not near snake
    while ((snake_pos - bait_pos) % width < 2 && (snake_pos - bait_pos) % width > -2) {
        bait_pos = rand() % (width * height);
    }
    board->cells[bait_pos] = char_bait;

    return board;
}


void print_initial_board(Board* board) {
    clear();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printw("%c  ", board->cells[indexOf(x, y)]);
        }
        printw("\n");
    }
    refresh();
}

void update_screen_position(const int x, const int y, const char c) {
    mvprintw(y, 3*x, "%c", c);
    refresh();
}

void spawn_new_bait(Board* board) {
    int bait_pos = rand() % (width * height);
    while (board->cells[bait_pos] != char_empty) {
        bait_pos = rand() % (width * height);
    }
    board->cells[bait_pos] = char_bait;

    int bait_x = bait_pos % width;
    int bait_y = bait_pos / width;
    update_screen_position(bait_x, bait_y, char_bait);
}

int direction(const int ch) {
    switch (ch) {
        case 'w': return 0;
        case 'd': return 1;
        case 's': return 2;
        case 'a': return 3;
        default: return -1;
    }
}

void move_snake_and_update_screen(Board* b, const int direction) {
    // if direction will be changed and new direction isn't opposite
    if (direction != -1 && (direction - b->snake_direction + 4) % 4 != 2) {
        b->snake_direction = direction;
    }

    const int next_x = (b->snake[b->snake_head_idx] % width) + (b->snake_direction == 1) - (b->snake_direction == 3);
    const int next_y = (b->snake[b->snake_head_idx] / width) + (b->snake_direction == 2) - (b->snake_direction == 0);
    const int next_head_pos = indexOf(next_x, next_y);
    const bool bait_eaten = b->cells[next_head_pos] == char_bait;

    // collision checks, we don't end the game in collision, just wait for new input
    if (next_x < 0 || next_x >= width || next_y < 0 || next_y >= height) // out of bounds
        return;
    if (b->cells[next_head_pos] == char_tail) // collision with tail
        return;

    const int prev_head_pos = b->snake[b->snake_head_idx];
    b->cells[prev_head_pos] = char_tail;
    update_screen_position(prev_head_pos % width, prev_head_pos / width, char_tail); // previous tail

    b->snake_head_idx = (b->snake_head_idx + 1) % (width * height); // update the index of head in snake array
    b->snake[b->snake_head_idx] = next_head_pos; // update the new head position
    b->cells[next_head_pos] = char_head;
    update_screen_position(next_x, next_y, char_head);

    // if a bait is eaten, just increase the snake length and spawn a new bait
    if (bait_eaten) {
        b->snake_length++;
        spawn_new_bait(b);
    }
    // if not, remove the last cell of tail
    else {
        int tail_end_index = (b->snake_head_idx - b->snake_length + width*height) % (width * height);
        int tail_end_position = b->snake[tail_end_index];
        b->cells[tail_end_position] = char_empty;
        int tail_x = tail_end_position % width;
        int tail_y = tail_end_position / width;
        update_screen_position(tail_x, tail_y, char_empty);
    }
}


void free_board(Board* b) {
    if (b != NULL) {
        free(b->cells);
        free(b->snake);
        free(b);
    }
}

void cleanup() {
    clear();
    endwin();
    free_board(board);
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

    board = create_initial_board();

    initscr();             // Start ncurses mode
    cbreak();              // Disable line buffering
    keypad(stdscr, TRUE);  // Enable arrow keys
    noecho();              // Don't display typed characters
    nodelay(stdscr, TRUE); // make getch non-blocking
    curs_set(0);           // Hide the cursor

    print_initial_board(board);

    const int fps = 10; // also affects the speed of the snake, BE CAREFUL
    const int frame_delay_ms = 1000 / fps; // delay between screen updates
    const int input_poll_ms = 20;
    int run = 1;

    struct timespec last_frame_time;
    clock_gettime(CLOCK_MONOTONIC, &last_frame_time);

    char last_valid_input = '\0';
    while (run) {

        char ch = getch();
        if (ch >= 'A' && ch <= 'Z')
            ch += 32;
        if (ch == 'q')
            run = 0;

        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time_ms = (current_time.tv_sec - last_frame_time.tv_sec) * 1000
                             + (current_time.tv_nsec - last_frame_time.tv_nsec) / 1000000;

        // if (direction(ch) != -1) {
        //     last_valid_input = direction(ch);
        // }
        // I discarded this idea because updating the frame whenever I get a new input feels smoother, even if it can make the game faster

        if (elapsed_time_ms >= frame_delay_ms || direction(ch) != -1) {
            move_snake_and_update_screen(board, direction(ch));
            last_frame_time = current_time;
        }

        napms(input_poll_ms); // sleep of ncurses used instead of sleep()
    }

    cleanup();
}



// the following code was abandoned when I discovered ncurses library

// struct termios orig_termios;
// void enable_raw_nonblocking_mode() {
//     struct termios term = orig_termios;
//
//     tcgetattr(STDIN_FILENO, &orig_termios); // Save the original terminal attributes
//
//     term.c_lflag &= ~(ECHO | ICANON); // Turn off echoing and canonical mode
//
//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &term); // apply the new settings
//
//     fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK); // enable non-blocking mode
// }
//
// void reset_terminal_settings() {
//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
// }
//
// void print_board(Board* board) {
//     char buffer[width * height * 3 + height + 1]; // char + 2 space for cells, +1 end character
//     int offset = 0;
//
//     clear_screen();
//
//     for (int y = 0; y < height; y++) {
//         for (int x = 0; x < width; x++) {
//             buffer[offset++] = board->cells[indexOf(x, y)];
//             buffer[offset++] = ' ';
//             buffer[offset++] = ' ';
//         }
//         buffer[offset++] = '\n';
//     }
//
//     buffer[offset] = '\0';
//     printf(buffer);
//     fflush(stdout);
//
//     // for saving to file
//     // FILE* f = fopen("snake.txt", "a");
//     // fprintf(f, buffer);
//     // fclose(f);
// }

