#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

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

/* top left corner: (0, 0)
* bottom right corner: (width-1, height-1) */
int indexOf(int x, int y) {
    return x + y * width;
}

Board* create_initial_board() {
    Board* board = malloc(sizeof(Board));
    if (board == NULL) {
        printf("Failed to allocate memory for the board\n");
        return NULL;
    }

    board->cells = (char*) malloc(sizeof(char*) * width * height);
    board->snake = (int*) malloc(sizeof(int) * width * height);
    board->snake_length = 1;

    // initial field of empty cells
    for (int i = 0; i < width * height; i++) {
        board->cells[i] = char_empty;
    }

    // put snake
    int snake_pos = rand() % (width * height);
    board->cells[snake_pos] = char_head;
    board->snake[0] = snake_pos;

    int snake_direction = rand() % 4;
    board->snake_direction = snake_direction;

    // put bait
    int bait_pos = rand() % (width * height);
    while (bait_pos == snake_pos) {
        bait_pos = rand() % (width * height);
    }
    board->cells[bait_pos] = char_bait;

    return board;
}

void spawn_new_bait(Board* board) {
    int bait_pos = rand() % (width * height);
    while (board->cells[bait_pos] != char_empty) {
        bait_pos = rand() % (width * height);
    }
    board->cells[bait_pos] = char_bait;
}


char read_key() {
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
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
    mvprintw(y, x, "%c", c);
    refresh();
}

void move_snake_and_update_screen(Board* b, const int direction) {
    int next_x = (b->snake[b->snake_head_idx] % width) + (direction == 1) - (direction == 3);
    int next_y = (b->snake[b->snake_head_idx] / width) + (direction == 2) - (direction == 0);
    int next_head_idx = indexOf(next_x, next_y);

    // collision checks, we don't end the game in collision, just wait for new input
    if (next_x < 0 || next_x >= width || next_y < 0 || next_y >= height) // out of bounds
        return;
    if (b->cells[next_head_idx] == char_tail) // collision with tail
        return;

    // if direction will be changed and new direction isn't opposite
    if (direction != -1 && (direction - b->snake_direction) % 4 != 2) {
        b->snake_direction = direction;
    }


}



void free_board(Board* b) {
    if (b != NULL) {
        free(b->cells);
        free(b->snake);
        free(b);
    }
}

void clear_screen() {
    system("clear");
}

void cleanup() {
    endwin();
    clear_screen();
}

// TODO
void handle_sigint(int sig) {
    cleanup();
    exit(0);
}
// TODO
void handle_sigterm(int sig) {
    cleanup();
    exit(0);
}

int main() {

    Board* board = create_initial_board();

    initscr();            // Start ncurses mode
    cbreak();             // Disable line buffering
    keypad(stdscr, TRUE); // Enable arrow keys
    noecho();             // Don't display typed characters
    curs_set(0);          // Hide the cursor

    print_initial_board(board);
    // sleep(10);

    const int frame_rate = 100;
    const int frame_delay_s = 1 / frame_rate;

    const int input_poll_ms = 20;

    int run = 1;
    while (run) {

        const char ch = read_key();
        if (ch == 'q')
            run = 0;

        move_snake(board, direction(ch));

        sleep(input_poll_ms / 1000);
    }

    cleanup();
    free(board);
    return 0;
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

