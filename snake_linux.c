#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define HEIGHT  20
#define WIDTH   40
#define FOOD    7

typedef struct snake {
	short row;
	short col;
	struct snake *next;
} snek;

void free_snek(snek *head);
void print_board(char board[HEIGHT][WIDTH]);
char wait_for_keypress(double milliseconds, char current_dir);
bool move_snek(snek* head, short row_nxt, short col_nxt, bool isfood);
void get_food_pos(short *pos_row, short *pos_col, char board[HEIGHT][WIDTH]);
void create_frame(snek *head, char curr_dir, short *food_pos_row, short *food_pos_col, char board[HEIGHT][WIDTH]);

int main(int argc, char *argv[]) {
	struct termios old_tio, new_tio;
	tcgetattr(STDIN_FILENO, &old_tio);

	new_tio = old_tio;
	new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

	char board[HEIGHT][WIDTH];
	snek *head = (snek *) malloc(sizeof(snek));
	snek *tail = (snek *) malloc(sizeof(snek));
	head->row = HEIGHT / 2 - 1, head->col = WIDTH / 2 - 1, head->next = tail;
	tail->row = HEIGHT / 2 - 1, tail->col = WIDTH / 2 - 2, tail->next = NULL;

	int snek_len = 2;
    char curr_dir = 'r';
    short food_pos_row;
    short food_pos_col;

	srand(time(NULL));
    get_food_pos(&food_pos_row, &food_pos_col, board);
    create_frame(head, curr_dir, &food_pos_row, &food_pos_col, board);
	printf("\e[1;1H\e[2J");
	print_board(board);
	
	while (true) {
        curr_dir = wait_for_keypress(75, curr_dir);
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
            printf("\e[H");
            if (head->row == food_pos_row && head->col == food_pos_col) {
                ++snek_len;
            }
            create_frame(head, curr_dir, &food_pos_row, &food_pos_col, board);
            print_board(board);
            printf("snake length = %d\n", snek_len);
            if (snek_len == HEIGHT * WIDTH) {
                printf("YOU WIN!!!\n");
                break;
            }
        } else {
            printf("GAME OVER!!!\n");
            break;
        }
    }

	tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

	free_snek(head);

	return 0;
}

void print_board(char board[HEIGHT][WIDTH]) {
	printf("\e[38;5;247m\e[48;5;242m╔");
	for (int i = 0; i < WIDTH; i++)
		printf("═");
	printf("╗\n");

    char *colors[] = {"\e[38;5;21m", "\e[38;5;51m"};
    char *bgcols[] = {"\e[48;5;22m", "\e[48;5;64m"};
    for (int i = 0; i < HEIGHT; i++) {
		printf("║");
		for (int j = 0; j < WIDTH; j++) {
            printf("%s", bgcols[(i+j)%2]);
			switch (board[i][j] & 0xf) {
                case 0:
                    printf(" ");
                    break;
                case 0x1:
                    printf("%s━", colors[(board[i][j] & 0x10) >> 4]);
                    break;
                case 0x2:
                    printf("%s┃", colors[(board[i][j] & 0x10) >> 4]);
                    break;
                case 0x3:
                    printf("%s┛", colors[(board[i][j] & 0x10) >> 4]);
                    break;
                case 0x4:
                    printf("%s┗", colors[(board[i][j] & 0x10) >> 4]);
                    break;
                case 0x5:
                    printf("%s┏", colors[(board[i][j] & 0x10) >> 4]);
                    break;
                case 0x6:
                    printf("%s┓", colors[(board[i][j] & 0x10) >> 4]);
                    break;
                case 0x7:
                    printf("\e[38;5;220m■");
                    break;
            }
		}
		printf("\e[38;5;247m\e[48;5;242m║\n");
	}

	printf("╚");
    for (int i = 0; i < WIDTH; i++)
            printf("═");
    printf("╝\e[0m\n");
}

bool move_snek(snek* head, short row_nxt, short col_nxt, bool isfood) {
    snek *seg_ptr = head;
    short temp_row, temp_col;
    while (seg_ptr->next) {
        temp_row = seg_ptr->row;
        temp_col = seg_ptr->col;
        seg_ptr->row = row_nxt;
        seg_ptr->col = col_nxt;
        row_nxt = temp_row;
        col_nxt = temp_col;
        seg_ptr = seg_ptr->next;
    }
    temp_row = seg_ptr->row;
    temp_col = seg_ptr->col;
    seg_ptr->row = row_nxt;
    seg_ptr->col = col_nxt;

    if (head->row < 0 || head->row >= HEIGHT || head->col < 0 || head->col >= WIDTH)
        return true;
    snek *ptr = head->next;
    while (ptr)
        if (head->row == ptr->row && head->col == ptr->col)
            return true;
        else
            ptr = ptr->next;

    if (isfood) {
        seg_ptr->next = (snek *) malloc(sizeof(snek));
        seg_ptr->next->row = temp_row;
        seg_ptr->next->col = temp_col;
        seg_ptr->next->next = NULL;
    }
    return false;
}

char wait_for_keypress(double milliseconds, char current_dir) {
    char new_dir = current_dir;
    double start = (double) clock() * 1000 / CLOCKS_PER_SEC, found;
	char c;
	while ((found = (double) clock() * 1000 / CLOCKS_PER_SEC) - start < milliseconds) {
		if (read(STDIN_FILENO, &c, 1) > 0 && c == '\033') { // Escape sequence for arrow keys
            read(STDIN_FILENO, &c, 1); // Read '['
            read(STDIN_FILENO, &c, 1); // Read the specific arrow key code
            if (c == 'D' && current_dir != 'l' && current_dir != 'r') {
                new_dir = 'l';
                break;
            } else if (c == 'C' && current_dir != 'l' && current_dir != 'r') {
                new_dir = 'r';
                break;
            } else if (c == 'A' && current_dir != 'u' && current_dir != 'd') {
                new_dir = 'u';
                break;
            } else if (c == 'B' && current_dir != 'u' && current_dir != 'd') {
                new_dir = 'd';
                break;
            }
        }
    }
	if (found - start < milliseconds)
        while ((double) clock() * 1000 / CLOCKS_PER_SEC - found < milliseconds - found + start);

    return new_dir;
}

void create_frame(snek *head, char curr_dir, short *food_pos_row, short *food_pos_col, char board[HEIGHT][WIDTH]) {
    for (int i = 0; i < HEIGHT; ++i)
        for (int j = 0; j < WIDTH; ++j)
            board[i][j] = 0;
    
    snek *prev = NULL;
    int alt = 0x10;
    while (head) {
        if (!prev) {
            if (curr_dir == 'l' || curr_dir == 'r')
                board[head->row][head->col] = 0x1; // ═
            else
                board[head->row][head->col] = 0x2; // ║
        } else if (!head->next) {
            if (head->row != prev->row)
                board[head->row][head->col] = 0x2;
            else
                board[head->row][head->col] = 0x1;
        } else {
            int vr1 = prev->row - head->row;
            int vc1 = prev->col - head->col;
            int vr2 = head->row - head->next->row;
            int vc2 = head->col - head->next->col;
            if (vr1 == 0 && vr2 == 0)
                board[head->row][head->col] = 0x1;
            else if (vc1 == 0 && vc2 == 0)
                board[head->row][head->col] = 0x2;
            else if (vr1 == -1 && vc2 == 1 || vc1 == -1 && vr2 == 1)
                board[head->row][head->col] = 0x3; // ╝
            else if (vr1 == -1 && vc2 == -1 || vc1 == 1 && vr2 == 1)
                board[head->row][head->col] = 0x4; // ╚
            else if (vc1 == 1 && vr2 == -1 || vr1 == 1 && vc2 == -1)
                board[head->row][head->col] = 0x5; // ╔
            else if (vc1 == -1 && vr2 == -1 || vr1 == 1 && vc2 == 1)
                board[head->row][head->col] = 0x6; // ╗
        }
        board[head->row][head->col] |= alt;
        alt ^= 0x10;
        prev = head;
        head = head->next;
    }
    
    if ((board[*food_pos_row][*food_pos_col] & 0xf) < 7 && (board[*food_pos_row][*food_pos_col] & 0xf) > 0)
        get_food_pos(food_pos_row, food_pos_col, board);
    board[*food_pos_row][*food_pos_col] = 0x7;
}

void free_snek(snek *head) {
    while (head) {
        snek *nxt = head->next;
        free(head);
        head = nxt;
    }
}

void get_food_pos(short *pos_row, short *pos_col, char board[HEIGHT][WIDTH]) {
    do {
        *pos_row = rand()%HEIGHT;
        *pos_col = rand()%WIDTH;
    } while ((board[*pos_row][*pos_col] & 0xf) < 7 && (board[*pos_row][*pos_col] & 0xf) > 0);
}