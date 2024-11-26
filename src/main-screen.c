#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <ncurses.h>
#include <sys/stat.h>


int is_game(const char* filename) {
    // check if it starts with "game_"
    char game_prefix[] = "game_";
    for (int i=0; i<5; i++) {
        if (game_prefix[i] != filename[i])
            return 0;
    }

    // add ./ to start
    char* path = malloc(sizeof(char) * (strlen(filename) + 3));
    strcpy(path, "./");
    strcat(path, filename);

    // check if it is an executable
    struct stat file_stat;
    if (stat(path, &file_stat) == -1) {
        perror("could not get file info");
        return 0;
    }
    // check if it is a regular file and has execute permission
    if ( !S_ISREG(file_stat.st_mode) || !(file_stat.st_mode & S_IXUSR)) {
        return 0;
    }

    return 1;
}

typedef struct {
    enum buttons {
        play = 0,
        quit = 1,
    } buttons;
    int current_button;
    int num_buttons;

    char** game_names;
    int current_game_idx;
    int num_games;
} MainScreen;

MainScreen* main_screen;

MainScreen* initialize_main_screen(char** game_names, int num_of_games) {
    main_screen = malloc(sizeof(MainScreen));
    main_screen->current_button = play;
    main_screen->num_buttons = 2;
    main_screen->game_names = game_names;
    main_screen->current_game_idx = 0;
    main_screen->num_games = num_of_games;
    return main_screen;
}

void update_screen_position(const int x, const int y, const char c) {
    mvprintw(y, x, "%c", c);
    refresh();
}

#define SELECT 1
#define DESELECT 0

void select_button(MainScreen* main_screen, const int button, const int select) {
    main_screen->current_button = button;

    int buttons_y = 7;
    int button_start_x = -1;
    int button_end_x = -1;
    if (button == 0) {
        button_start_x = 4;
        button_end_x = 8;
    }
    else if (button == 1) {
        button_start_x = 12;
        button_end_x = 16;
    }

    if (button_start_x == -1) {
        printf("error selecting button");
        return;
    }

    if (select == SELECT) {
        update_screen_position(button_start_x - 1, buttons_y, '[');
        update_screen_position(button_end_x, buttons_y, ']');
    }
    else if (select == DESELECT) {
        update_screen_position(button_start_x - 1, buttons_y, ' ');
        update_screen_position(button_end_x, buttons_y, ' ');
    }
}

void print_whole_screen(MainScreen* main_screen) {
    clear();
    printw("=== Virtual Game Console ===\n");
    printw("Use keys a and d to select button\n");
    printw("Use keys w and s to change game\n");
    printw("Press enter to select\n");
    printw("Press q to quit\n");
    printw("\n");

    printw("        Current game: %s\n", main_screen->game_names[main_screen->current_game_idx]);
    printw("    play    quit    \n");
    select_button(main_screen, main_screen->current_button, SELECT);

    refresh();
}

void free_game_names(char** game_names, int num_of_games) {
    for (int i=0; i<num_of_games; i++) {
        free(game_names[i]);
    }
    free(game_names);
}

char** game_names;
int num_of_games;

void free_main_screen(MainScreen* main_screen) {
    free_game_names(main_screen->game_names, main_screen->num_games);
    free(main_screen);
}

void cleanup() {
    free_main_screen(main_screen);
    endwin();
    system("clear");
}
void handle_sigint(int sig) {
    cleanup();
    exit(0);
}
void handle_sigterm(int sig) {
    cleanup();
    exit(0);
}

void slide_game(MainScreen* main_screen, int direction) {
    int prev_idx = main_screen->current_game_idx;
    int num_games = main_screen->num_games;
    int next_game_idx = (prev_idx + direction + num_games) % num_games;

    main_screen->current_game_idx = next_game_idx;

    int game_names_y = 6;
    int game_name_start = 22;
    int game_name_end = 22 + strlen(main_screen->game_names[prev_idx]);

    int i = 0;
    while (i < strlen(main_screen->game_names[next_game_idx])) {
        update_screen_position(game_name_start + i, game_names_y, main_screen->game_names[next_game_idx][i]);
        i++;
    }
    while (i <= game_name_end) {
        update_screen_position(game_name_start + i, game_names_y, ' ');
        i++;
    }
}

void init_ncurses() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
}

void start_game(MainScreen* main_screen) {
    int game = main_screen->current_game_idx;
    // execute the game executable on the current directory as a child process
    char* game_name = main_screen->game_names[game];
    char* command = malloc(sizeof(char) * (strlen(game_name) + 3));
    strcpy(command, "./");
    strcat(command, game_name);

    system(command); // runs the game as a child and waits for it to finish

    curs_set(0); // WHY DOESN'T THIS WORK

    free(command);
    init_ncurses();
    refresh();
    print_whole_screen(main_screen);
}

void handle_input(MainScreen* main_screen, char ch) {
    switch (ch) {
        case 'w': // slide to next game
            slide_game(main_screen, 1);
            break;
        case 's': // slide to previous game
            slide_game(main_screen, -1);
            break;
        case 'a':
            select_button(main_screen, quit, DESELECT);
            select_button(main_screen, play, SELECT);
            break;
        case 'd':
            select_button(main_screen, play, DESELECT);
            select_button(main_screen, quit, SELECT);
            break;
        case '\n': // enter
            if (main_screen->current_button == play) {
                start_game(main_screen);
            }
            else if (main_screen->current_button == quit) {
                cleanup();
                exit(0);
            }
            break;
        default:
            return;
    }
}


int main() {
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigterm);

    num_of_games = 0;

    DIR* current_directory = opendir(".");
    if (current_directory == NULL) {
        printf("Unable to open current directory\n");
        return 1;
    }

    struct dirent* dir_entry;

    // get the number of games
    while ((dir_entry = readdir(current_directory)) != NULL) {
        if (is_game(dir_entry->d_name)) {
            num_of_games++;
        }
    }
    rewinddir(current_directory);

    game_names = malloc(sizeof(char*) * num_of_games);

    // get the names of the games
    int i = 0;
    while ((dir_entry = readdir(current_directory)) != NULL) {
        if (is_game(dir_entry->d_name)) {
            game_names[i] = malloc(sizeof(char) * strlen(dir_entry->d_name) + 1); // +1 for the \0
            strcpy(game_names[i], dir_entry->d_name);
            i++;
        }
    }

    init_ncurses();

    main_screen = initialize_main_screen(game_names, num_of_games);
    print_whole_screen(main_screen);

    int run = 1;
    const int input_poll_ms = 20;
    while (run) {

        char ch = getch();
        if (ch >= 'A' && ch <= 'Z')
            ch += 32;
        if (ch == 'q')
            run = 0;

        if (ch == 'w' || ch == 's' || ch == 'a' || ch == 'd' || ch == '\n') {
            handle_input(main_screen, ch);
        }

        napms(input_poll_ms);
    }

    cleanup();
    return 0;
}
