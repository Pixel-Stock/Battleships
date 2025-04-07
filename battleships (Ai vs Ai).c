#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// -----------------------------------------------------------------------------
// Configuration & Global Constants
// -----------------------------------------------------------------------------

#define BOARD_SIZE 10
static const char ALPHABET[] = "ABCDEFGHIJ";

// Ship sizes: one ship of size 5, one of size 3, two of size 2, one of size 1.
static const int SHIP_SIZES[] = {5, 3, 2, 2, 1};
static const int NUM_SHIPS = sizeof(SHIP_SIZES) / sizeof(SHIP_SIZES[0]);

// -----------------------------------------------------------------------------
// AI Definitions
// -----------------------------------------------------------------------------

typedef enum {
    HUNT_MODE,
    TARGET_MODE
} AImode;

typedef struct {
    AImode mode;
    int last_hit_x;
    int last_hit_y;
    int target_candidates[4][2]; // For standard AI target mode.
    int num_candidates;
} AIState;

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------

// Board functions
void initialize_board(char board[BOARD_SIZE][BOARD_SIZE]);
void print_board(const char board[BOARD_SIZE][BOARD_SIZE], bool reveal_ships);
bool place_ship(char board[BOARD_SIZE][BOARD_SIZE], int size, bool horizontal, int x, int y);
bool place_ships_random(char board[BOARD_SIZE][BOARD_SIZE]);
bool process_attack(char board[BOARD_SIZE][BOARD_SIZE], int x, int y);
bool check_victory(const char board[BOARD_SIZE][BOARD_SIZE]);
void update_board_for_destroyed_ship(char board[BOARD_SIZE][BOARD_SIZE]);

// Flood-fill helper for ship update
void floodFillShip(char board[BOARD_SIZE][BOARD_SIZE],
                   int i, int j,
                   bool visited[BOARD_SIZE][BOARD_SIZE],
                   int group[][2],
                   int *groupCount,
                   bool *hasIntact);

// Manual ship placement (for player input)
void manual_place_ships(char board[BOARD_SIZE][BOARD_SIZE], const char *playerName);

// Standard AI functions (Easy/Medium)
void initialize_ai(AIState *state);
void add_target_candidates(AIState *state, int x, int y, char board[BOARD_SIZE][BOARD_SIZE]);
void ai_attack(AIState *state, char player_board[BOARD_SIZE][BOARD_SIZE]);

// Nightmare mode AI (Hard mode)
void nightmare_ai_attack(AIState *state, char player_board[BOARD_SIZE][BOARD_SIZE], char ai_guess[BOARD_SIZE][BOARD_SIZE]);

// Utility functions
void display_rules();
void player_attack(char opponent_board[BOARD_SIZE][BOARD_SIZE],
                   char guess_board[BOARD_SIZE][BOARD_SIZE],
                   const char *player_name);

void wait_for_enter();

// -----------------------------------------------------------------------------
// Board Function Implementations
// -----------------------------------------------------------------------------

void initialize_board(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            board[i][j] = '.';
}

void print_board(const char board[BOARD_SIZE][BOARD_SIZE], bool reveal_ships) {
    printf("   A B C D E F G H I J\n");
    printf("  ---------------------\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d| ", i + 1);
        for (int j = 0; j < BOARD_SIZE; j++) {
            char cell = board[i][j];
            if (reveal_ships) {
                if (cell == '&' || cell == '#' || cell == '0' || cell == 'x')
                    printf("%c ", cell);
                else
                    printf(". ");
            } else {
                // In hidden mode, intact ships are not shown.
                if (cell == '#' || cell == '0' || cell == 'x')
                    printf("%c ", cell);
                else
                    printf(". ");
            }
        }
        printf("\n");
    }
}

bool place_ship(char board[BOARD_SIZE][BOARD_SIZE], int size, bool horizontal, int x, int y) {
    if (horizontal) {
        if (y + size > BOARD_SIZE) return false;
        for (int i = 0; i < size; i++) {
            if (board[x][y + i] != '.') return false;
        }
        for (int i = 0; i < size; i++) {
            board[x][y + i] = '&';
        }
    } else {
        if (x + size > BOARD_SIZE) return false;
        for (int i = 0; i < size; i++) {
            if (board[x + i][y] != '.') return false;
        }
        for (int i = 0; i < size; i++) {
            board[x + i][y] = '&';
        }
    }
    return true;
}

bool place_ships_random(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < NUM_SHIPS; i++) {
        bool placed = false;
        int attempts = 0;
        while (!placed && attempts < 1000) {
            int x = rand() % BOARD_SIZE;
            int y = rand() % BOARD_SIZE;
            bool horizontal = (rand() % 2) == 0;
            placed = place_ship(board, SHIP_SIZES[i], horizontal, x, y);
            attempts++;
        }
    }
    return true;
}

bool process_attack(char board[BOARD_SIZE][BOARD_SIZE], int x, int y) {
    if (board[x][y] == '&') {
        board[x][y] = '#';
        update_board_for_destroyed_ship(board);
        return true;
    } else if (board[x][y] == '.') {
        board[x][y] = 'x';
        return false;
    }
    return false;
}

bool check_victory(const char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] == '&')
                return false;
    return true;
}

void floodFillShip(char board[BOARD_SIZE][BOARD_SIZE],
                   int i, int j,
                   bool visited[BOARD_SIZE][BOARD_SIZE],
                   int group[][2],
                   int *groupCount,
                   bool *hasIntact) {
    if (i < 0 || i >= BOARD_SIZE || j < 0 || j >= BOARD_SIZE)
        return;
    if (visited[i][j])
        return;
    if (board[i][j] != '#' && board[i][j] != '&')
        return;

    visited[i][j] = true;
    group[*groupCount][0] = i;
    group[*groupCount][1] = j;
    (*groupCount)++;
    if (board[i][j] == '&')
        *hasIntact = true;

    floodFillShip(board, i - 1, j, visited, group, groupCount, hasIntact);
    floodFillShip(board, i + 1, j, visited, group, groupCount, hasIntact);
    floodFillShip(board, i, j - 1, visited, group, groupCount, hasIntact);
    floodFillShip(board, i, j + 1, visited, group, groupCount, hasIntact);
}

void update_board_for_destroyed_ship(char board[BOARD_SIZE][BOARD_SIZE]) {
    bool visited[BOARD_SIZE][BOARD_SIZE] = { false };
    int group[BOARD_SIZE * BOARD_SIZE][2];
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (!visited[i][j] && (board[i][j] == '#' || board[i][j] == '&')) {
                int groupCount = 0;
                bool hasIntact = false;
                floodFillShip(board, i, j, visited, group, &groupCount, &hasIntact);
                if (!hasIntact && groupCount > 0) {
                    for (int k = 0; k < groupCount; k++) {
                        int gx = group[k][0];
                        int gy = group[k][1];
                        board[gx][gy] = '0';
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Manual Ship Placement
// -----------------------------------------------------------------------------

void manual_place_ships(char board[BOARD_SIZE][BOARD_SIZE], const char *playerName) {
    printf("\n%s, place your ships on the board.\n", playerName);
    for (int i = 0; i < NUM_SHIPS; i++) {
        int size = SHIP_SIZES[i];
        bool placed = false;
        while (!placed) {
            print_board(board, true);
            printf("Place your ship of size %d.\n", size);
            printf("Enter starting coordinate (e.g., A1): ");
            char coord[5];
            scanf("%s", coord);
            char col = coord[0];
            int row = atoi(&coord[1]);
            if (row < 1 || row > BOARD_SIZE || col < 'A' || col > 'A' + BOARD_SIZE - 1) {
                printf("Invalid coordinate. Try again.\n");
                continue;
            }
            int x = row - 1;
            int y = col - 'A';
            printf("Enter orientation (H for horizontal, V for vertical): ");
            char orient;
            scanf(" %c", &orient);
            bool horizontal;
            if (orient == 'H' || orient == 'h')
                horizontal = true;
            else if (orient == 'V' || orient == 'v')
                horizontal = false;
            else {
                printf("Invalid orientation. Use H or V.\n");
                continue;
            }
            if (!place_ship(board, size, horizontal, x, y)) {
                printf("Invalid placement or overlapping ship. Try again.\n");
            } else {
                placed = true;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Standard AI Functionality (Easy/Medium)
// -----------------------------------------------------------------------------

void initialize_ai(AIState *state) {
    state->mode = HUNT_MODE;
    state->last_hit_x = -1;
    state->last_hit_y = -1;
    state->num_candidates = 0;
}

void add_target_candidates(AIState *state, int x, int y, char board[BOARD_SIZE][BOARD_SIZE]) {
    int directions[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
    for (int d = 0; d < 4; d++) {
        int nx = x + directions[d][0];
        int ny = y + directions[d][1];
        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
            if (board[nx][ny] == '.' || board[nx][ny] == '&') {
                bool duplicate = false;
                for (int i = 0; i < state->num_candidates; i++) {
                    if (state->target_candidates[i][0] == nx && state->target_candidates[i][1] == ny) {
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate && state->num_candidates < 4) {
                    state->target_candidates[state->num_candidates][0] = nx;
                    state->target_candidates[state->num_candidates][1] = ny;
                    state->num_candidates++;
                }
            }
        }
    }
}

void ai_attack(AIState *state, char player_board[BOARD_SIZE][BOARD_SIZE]) {
    int x, y;
    bool hit;
    if (state->mode == HUNT_MODE) {
        do {
            x = rand() % BOARD_SIZE;
            y = rand() % BOARD_SIZE;
        } while (player_board[x][y] == 'x' || player_board[x][y] == '#' || player_board[x][y] == '0');

        hit = process_attack(player_board, x, y);
        if (hit) {
            printf("Computer HIT at %c%d!\n", ALPHABET[y], x + 1);
            state->mode = TARGET_MODE;
            state->last_hit_x = x;
            state->last_hit_y = y;
            state->num_candidates = 0;
            add_target_candidates(state, x, y, player_board);
        } else {
            printf("Computer MISSED at %c%d!\n", ALPHABET[y], x + 1);
        }
    } else { // TARGET_MODE
        if (state->num_candidates > 0) {
            int idx = state->num_candidates - 1;
            x = state->target_candidates[idx][0];
            y = state->target_candidates[idx][1];
            state->num_candidates--;
            hit = process_attack(player_board, x, y);
            if (hit) {
                printf("Computer HIT at %c%d!\n", ALPHABET[y], x + 1);
                state->last_hit_x = x;
                state->last_hit_y = y;
                add_target_candidates(state, x, y, player_board);
            } else {
                printf("Computer MISSED at %c%d!\n", ALPHABET[y], x + 1);
            }
        } else {
            state->mode = HUNT_MODE;
            ai_attack(state, player_board);
        }
        if (state->last_hit_x >= 0 && state->last_hit_y >= 0 &&
            player_board[state->last_hit_x][state->last_hit_y] == '0') {
            state->mode = HUNT_MODE;
            state->num_candidates = 0;
            state->last_hit_x = -1;
            state->last_hit_y = -1;
        }
    }
}

// -----------------------------------------------------------------------------
// Nightmare Mode AI (Hard)
// -----------------------------------------------------------------------------

// This function uses a separate AI guess board (ai_guess) to compute a probability
// density map and choose the best cell. It also falls back to target adjacent to a hit.
void nightmare_ai_attack(AIState *state, char player_board[BOARD_SIZE][BOARD_SIZE], char ai_guess[BOARD_SIZE][BOARD_SIZE]) {
    int i, j;
    bool hit = false;
    // Check for adjacent target cells from a previous hit.
    bool targetFound = false;
    int target_x = -1, target_y = -1;
    for (i = 0; i < BOARD_SIZE && !targetFound; i++) {
        for (j = 0; j < BOARD_SIZE && !targetFound; j++) {
            if (ai_guess[i][j] == '#') {
                int dirs[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
                for (int d = 0; d < 4; d++) {
                    int nx = i + dirs[d][0];
                    int ny = j + dirs[d][1];
                    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
                        if (ai_guess[nx][ny] == '.') {
                            targetFound = true;
                            target_x = nx;
                            target_y = ny;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (targetFound) {
        hit = process_attack(player_board, target_x, target_y);
        if (hit) {
            printf("Computer (Nightmare) HIT at %c%d!\n", ALPHABET[target_y], target_x + 1);
            ai_guess[target_x][target_y] = '#';
        } else {
            printf("Computer (Nightmare) MISSED at %c%d!\n", ALPHABET[target_y], target_x + 1);
            ai_guess[target_x][target_y] = 'x';
        }
        return;
    }

    // Compute a probability density map for each untried cell.
    int prob[BOARD_SIZE][BOARD_SIZE] = {0};
    for (int s = 0; s < NUM_SHIPS; s++) {
        int shipSize = SHIP_SIZES[s];
        // Horizontal placements.
        for (i = 0; i < BOARD_SIZE; i++) {
            for (j = 0; j <= BOARD_SIZE - shipSize; j++) {
                bool valid = true;
                for (int k = 0; k < shipSize; k++) {
                    char cell = ai_guess[i][j+k];
                    if (cell == 'x' || cell == '0') {
                        valid = false;
                        break;
                    }
                }
                if (valid) {
                    for (int k = 0; k < shipSize; k++) {
                        prob[i][j+k]++;
                    }
                }
            }
        }
        // Vertical placements.
        for (j = 0; j < BOARD_SIZE; j++) {
            for (i = 0; i <= BOARD_SIZE - shipSize; i++) {
                bool valid = true;
                for (int k = 0; k < shipSize; k++) {
                    char cell = ai_guess[i+k][j];
                    if (cell == 'x' || cell == '0') {
                        valid = false;
                        break;
                    }
                }
                if (valid) {
                    for (int k = 0; k < shipSize; k++) {
                        prob[i+k][j]++;
                    }
                }
            }
        }
    }
    // Choose the cell with the highest probability.
    int maxProb = -1, best_i = -1, best_j = -1;
    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            if (ai_guess[i][j] == '.' && prob[i][j] > maxProb) {
                maxProb = prob[i][j];
                best_i = i;
                best_j = j;
            }
        }
    }
    if (best_i >= 0 && best_j >= 0) {
        hit = process_attack(player_board, best_i, best_j);
        if (hit) {
            printf("Computer (Nightmare) HIT at %c%d!\n", ALPHABET[best_j], best_i + 1);
            ai_guess[best_i][best_j] = '#';
        } else {
            printf("Computer (Nightmare) MISSED at %c%d!\n", ALPHABET[best_j], best_i + 1);
            ai_guess[best_i][best_j] = 'x';
        }
    }
}

// -----------------------------------------------------------------------------
// Utility Functions
// -----------------------------------------------------------------------------

void display_rules() {
    printf("Welcome to Battleship!\n\n");
    printf("Game Rules:\n");
    printf("1. Players take turns attacking each other's ships.\n");
    printf("2. Ships are placed on a 10x10 grid.\n");
    printf("3. The first player to sink all enemy ships wins.\n\n");
    printf("Symbols on the Board:\n");
    printf("  '.' - Water\n");
    printf("  '&' - Intact ship part\n");
    printf("  '#' - Hit ship part\n");
    printf("  '0' - Destroyed ship\n");
    printf("  'x' - Missed attack\n\n");
}

void player_attack(char opponent_board[BOARD_SIZE][BOARD_SIZE],
                   char guess_board[BOARD_SIZE][BOARD_SIZE],
                   const char *player_name) {
    char move[5];
    int x, y;
    while (1) {
        printf("%s, enter attack coordinates (e.g., A1, A10): ", player_name);
        scanf("%s", move);
        char col = move[0];
        int row = atoi(&move[1]);
        if (row < 1 || row > BOARD_SIZE || col < 'A' || col > 'A' + BOARD_SIZE - 1) {
            printf("Invalid coordinates. Try again.\n");
            continue;
        }
        x = row - 1;
        y = col - 'A';
        if (guess_board[x][y] == 'x' || guess_board[x][y] == '#' || guess_board[x][y] == '0') {
            printf("Already attacked this position. Try again.\n");
            continue;
        }
        bool hit = process_attack(opponent_board, x, y);
        if (hit) {
            guess_board[x][y] = '#';
            printf("You HIT at %c%d!\n", col, row);
        } else {
            guess_board[x][y] = 'x';
            printf("You MISSED at %c%d!\n", col, row);
        }
        break;
    }
}

void wait_for_enter() {
    printf("Press Enter to continue...");
    while(getchar()!='\n'); // flush any leftover newline
    getchar();
}

// -----------------------------------------------------------------------------
// main()
// -----------------------------------------------------------------------------

int main() {
    srand(time(NULL));
    display_rules();

    printf("Choose mode: (1) Player vs Player  (2) Player vs Computer  (3) NIGHTMARE MODE  (4) Computer vs Computer: ");
    char mode;
    scanf(" %c", &mode);
    while (mode != '1' && mode != '2' && mode != '3' && mode != '4') {
        printf("Invalid choice! Please choose again: ");
        scanf(" %c", &mode);
    }

    // Initialize boards.
    char player1_board[BOARD_SIZE][BOARD_SIZE];
    char player2_board[BOARD_SIZE][BOARD_SIZE];
    char player1_guess_board[BOARD_SIZE][BOARD_SIZE];
    char player2_guess_board[BOARD_SIZE][BOARD_SIZE]; // For PvP.
    initialize_board(player1_board);
    initialize_board(player2_board);
    initialize_board(player1_guess_board);
    initialize_board(player2_guess_board);

    // For PvP mode, both players place ships.
    // For PvC, Nightmare mode, and Computer vs Computer, the player places ships on one board,
    // and the computer's board is randomly generated.
    if (mode == '1') {
        manual_place_ships(player1_board, "Player 1");
        manual_place_ships(player2_board, "Player 2");
    } else {
        manual_place_ships(player1_board, "Player");
        place_ships_random(player2_board);
    }

    // Setup AI state for modes 2, 3, and 4.
    AIState ai_state1, ai_state2;
    if (mode == '2' || mode == '3' || mode == '4') {
        initialize_ai(&ai_state1);
    }
    if (mode == '4') {
        initialize_ai(&ai_state2);
    }

    if (mode == '1') {  // Player vs Player
        char current_turn = '1';
        while (true) {
            if (current_turn == '1') {
                printf("\n--- Player 1's Turn ---\n");
                print_board(player1_board, true);
                print_board(player2_guess_board, false);
                player_attack(player2_board, player2_guess_board, "Player 1");
                if (check_victory(player2_board)) {
                    printf("Player 1 wins!\n");
                    break;
                }
                current_turn = '2';
            } else {
                printf("\n--- Player 2's Turn ---\n");
                print_board(player2_board, true);
                print_board(player1_guess_board, false);
                player_attack(player1_board, player1_guess_board, "Player 2");
                if (check_victory(player1_board)) {
                    printf("Player 2 wins!\n");
                    break;
                }
                current_turn = '1';
            }
        }
    } else if (mode == '2') {  // Player vs Computer (Normal)
        while (true) {
            printf("\n--- Player's Turn ---\n");
            print_board(player1_board, true);
            print_board(player2_board, false);
            player_attack(player2_board, player2_board, "Player");
            if (check_victory(player2_board)) {
                printf("Player wins!\n");
                break;
            }
            printf("\n--- Computer's Turn ---\n");
            ai_attack(&ai_state1, player1_board);
            printf("Your board after computer attack:\n");
            print_board(player1_board, true);
            if (check_victory(player1_board)) {
                printf("Computer wins!\n");
                break;
            }
            wait_for_enter();
        }
    } else if (mode == '3') {  // Nightmare Mode (Player vs Computer using Nightmare AI)
        // Create separate AI guess board for nightmare AI.
        char ai_guess[BOARD_SIZE][BOARD_SIZE];
        initialize_board(ai_guess);
        while (true) {
            printf("\n--- Player's Turn ---\n");
            print_board(player1_board, true);
            print_board(player2_board, false);
            player_attack(player2_board, player2_board, "Player");
            if (check_victory(player2_board)) {
                printf("Player wins!\n");
                break;
            }
            printf("\n--- Computer's (Nightmare) Turn ---\n");
            nightmare_ai_attack(&ai_state1, player1_board, ai_guess);
            printf("Your board after computer attack:\n");
            print_board(player1_board, true);
            if (check_victory(player1_board)) {
                printf("Computer wins!\n");
                break;
            }
            wait_for_enter();
        }
    } else if (mode == '4') {  // Computer vs Computer (Nightmare vs Nightmare)
        // Create two boards and two separate AI guess boards.
        char comp1_board[BOARD_SIZE][BOARD_SIZE];
        char comp2_board[BOARD_SIZE][BOARD_SIZE];
        char comp1_guess[BOARD_SIZE][BOARD_SIZE];
        char comp2_guess[BOARD_SIZE][BOARD_SIZE];
        initialize_board(comp1_board);
        initialize_board(comp2_board);
        initialize_board(comp1_guess);
        initialize_board(comp2_guess);
        // Randomly place ships on both computer boards.
        place_ships_random(comp1_board);
        place_ships_random(comp2_board);
        while (true) {
            printf("\n--- Computer 1's (Nightmare) Turn ---\n");
            nightmare_ai_attack(&ai_state1, comp2_board, comp1_guess);
            printf("Computer 2's board after attack:\n");
            print_board(comp2_board, false);
            if (check_victory(comp2_board)) {
                printf("Computer 1 wins!\n");
                break;
            }
            wait_for_enter();

            printf("\n--- Computer 2's (Nightmare) Turn ---\n");
            nightmare_ai_attack(&ai_state2, comp1_board, comp2_guess);
            printf("Computer 1's board after attack:\n");
            print_board(comp1_board, false);
            if (check_victory(comp1_board)) {
                printf("Computer 2 wins!\n");
                break;
            }
            wait_for_enter();
        }
    }

    return 0;
}
