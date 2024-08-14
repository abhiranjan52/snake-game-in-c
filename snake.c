#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#define WIDTH 60
#define HEIGHT 30
#define SNEK_SEGMENT 0xdb
#define FOOD 0xfe

typedef struct snek {
    short row;
    short col;
    struct snek *behind;
} SNEK;

void gotoxy(COORD c);
void free_snek(SNEK *head);
void print_board(unsigned char (*)[WIDTH]);
COORD getxy(HANDLE hConsoleOutput);
char wait_for_keypress(double milliseconds, char current_dir);
bool move_snek(SNEK* head, short row_nxt, short col_nxt, bool isfood);
void get_food_pos(short *pos_row, short *pos_col, unsigned char (*board)[WIDTH]);
void create_frame(SNEK *head, short *food_pos_row, short *food_pos_col, unsigned char (*board)[WIDTH]);

int main(int argc, char *argv[]) {
    unsigned char board[HEIGHT][WIDTH];
    SNEK *head = (SNEK *) malloc(sizeof(SNEK));
    head->row = HEIGHT/2;
    head->col = WIDTH/ 2;
    head->behind = (SNEK *) malloc(sizeof(SNEK));
    head->behind->row = HEIGHT/2;
    head->behind->col = WIDTH/2-1;
    head->behind->behind = NULL;

    int snek_len = 2;
    char curr_dir = 'r';
    short food_pos_row;
    short food_pos_col;

    srand(time(NULL));
    get_food_pos(&food_pos_row, &food_pos_col, board);
    create_frame(head, &food_pos_row, &food_pos_col, board);
    COORD coord = getxy(GetStdHandle(STD_OUTPUT_HANDLE));
    print_board(board);

    while (true) {
        curr_dir = wait_for_keypress(50, curr_dir);
        if (!curr_dir) {
            printf("RSHIFT pressed. Aborting.\n");
            break;
        }
        bool collision;
        switch (curr_dir) {
            case 'l':
                collision = move_snek(head, head->row, head->col-1, board[head->row][head->col-1] == FOOD);
                break;
            case 'r':
                collision = move_snek(head, head->row, head->col+1, board[head->row][head->col+1] == FOOD);
                break;
            case 'u':
                collision = move_snek(head, head->row-1, head->col, board[head->row-1][head->col] == FOOD);
                break;
            case 'd':
                collision = move_snek(head, head->row+1, head->col, board[head->row+1][head->col] == FOOD);
                break;
        }
        if (!collision) {
            gotoxy(coord);
            if (head->row == food_pos_row && head->col == food_pos_col) {
                Beep(750, 50);
                ++snek_len;
            }
            create_frame(head, &food_pos_row, &food_pos_col, board);
            print_board(board);
            printf("snake length = %d\n", snek_len);
            if (snek_len == HEIGHT*WIDTH) {
                printf("YOU WIN!!!\n");
                break;
            }
        } else {
            printf("GAME OVER!!!\n");
            break;
        }
    }
    free_snek(head);

    return 0;
}
void print_board(unsigned char (*board)[WIDTH]) {
    putchar(0xc9);
    for (int i = 0; i < WIDTH; ++i)
        putchar(0xcd);
    putchar(0xbb);
    putchar('\n');
    for (int i = 0; i < HEIGHT; ++i) {
        putchar(0xba);
        for (int j = 0; j < WIDTH; ++j)
            putchar(board[i][j]);
        putchar(0xba);
        putchar('\n');
    }
    putchar(0xc8);
    for (int i = 0; i < WIDTH; ++i)
        putchar(0xcd);
    putchar(0xbc);
    putchar('\n');
}
bool move_snek(SNEK* head, short row_nxt, short col_nxt, bool isfood) {
    SNEK *seg_ptr = head;
    short temp_row, temp_col;
    while (seg_ptr->behind) {
        temp_row = seg_ptr->row;
        temp_col = seg_ptr->col;
        seg_ptr->row = row_nxt;
        seg_ptr->col = col_nxt;
        row_nxt = temp_row;
        col_nxt = temp_col;
        seg_ptr = seg_ptr->behind;
    }
    temp_row = seg_ptr->row;
    temp_col = seg_ptr->col;
    seg_ptr->row = row_nxt;
    seg_ptr->col = col_nxt;

    if (head->row < 0 || head->row >= HEIGHT || head->col < 0 || head->col >= WIDTH)
        return true;
    SNEK *ptr = head->behind;
    while (ptr)
        if (head->row == ptr->row && head->col == ptr->col)
            return true;
        else
            ptr = ptr->behind;

    if (isfood) {
        seg_ptr->behind = (SNEK *) malloc(sizeof(SNEK));
        seg_ptr->behind->row = temp_row;
        seg_ptr->behind->col = temp_col;
        seg_ptr->behind->behind = NULL;
    }
    return false;
}
char wait_for_keypress(double milliseconds, char current_dir) {
    char new_dir = current_dir;
    double start = (double) clock(), found;
    while ((found = (double) clock()) - start < milliseconds)
        if ((GetKeyState(VK_LEFT) & 0x8000) && current_dir != 'l' && current_dir != 'r') {
            new_dir = 'l';
            break;
        } else if ((GetKeyState(VK_RIGHT) & 0x8000) && current_dir != 'l' && current_dir != 'r') {
            new_dir = 'r';
            break;
        } else if ((GetKeyState(VK_UP) & 0x8000) && current_dir != 'u' && current_dir != 'd') {
            new_dir = 'u';
            break;
        } else if ((GetKeyState(VK_DOWN) & 0x8000) && current_dir != 'u' && current_dir != 'd') {
            new_dir = 'd';
            break;
        } else if (GetKeyState(VK_RSHIFT) & 0x8000) {
            new_dir = '\0';
            break;
        }
    if (found - start < milliseconds)
        while ((double) clock() - found < milliseconds - found + start);

    return new_dir;
}
void create_frame(SNEK *head, short *food_pos_row, short *food_pos_col, unsigned char (*board)[WIDTH]) {
    for (int i = 0; i < HEIGHT; ++i)
        for (int j = 0; j < WIDTH; ++j)
            board[i][j] = ' ';
    while (head) {
        board[head->row][head->col] = SNEK_SEGMENT;
        head = head->behind;
    }
    if (board[*food_pos_row][*food_pos_col] == SNEK_SEGMENT)
        get_food_pos(food_pos_row, food_pos_col, board);
    board[*food_pos_row][*food_pos_col] = FOOD;
}
void free_snek(SNEK *head) {
    while (head) {
        SNEK *nxt = head->behind;
        free(head);
        head = nxt;
    }
}
void get_food_pos(short *pos_row, short *pos_col, unsigned char (*board)[WIDTH]) {
    do {
        *pos_row = rand()%HEIGHT;
        *pos_col = rand()%WIDTH;
    } while (board[*pos_row][*pos_col] == SNEK_SEGMENT);
}
void gotoxy(COORD c) {
    static HANDLE h = NULL;
    if(!h)
        h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(h,c);
}
COORD getxy(HANDLE hConsoleOutput) {
    CONSOLE_SCREEN_BUFFER_INFO cbsi;
    if (GetConsoleScreenBufferInfo(hConsoleOutput, &cbsi)) {
        return cbsi.dwCursorPosition;
    } else {
        COORD invalid = {0, 0};
        return invalid;
    }
}
