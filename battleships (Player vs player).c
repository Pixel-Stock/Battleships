#include <stdio.h>
#include <stdlib.h>

#define GRID_SIZE 8
#define NUM_SHIPS 5

// Function prototypes
void initializeGrid(char grid[GRID_SIZE][GRID_SIZE]);
void displayGrid(char grid[GRID_SIZE][GRID_SIZE], int revealShips);
void placeShips(char grid[GRID_SIZE][GRID_SIZE]);
int makeMove(char grid[GRID_SIZE][GRID_SIZE], int row, int col);
int isGameOver(char grid[GRID_SIZE][GRID_SIZE]);

int main() {
    char player1Grid[GRID_SIZE][GRID_SIZE];
    char player2Grid[GRID_SIZE][GRID_SIZE];
    int row, col, result;
    int currentPlayer = 1;

    // Places the grids
    initializeGrid(player1Grid);
    initializeGrid(player2Grid);

    // Players place the ships
    printf("Player 1, place your ships:\n");
    placeShips(player1Grid);
    printf("Player 2, place your ships:\n");
    placeShips(player2Grid);

    printf("\nWelcome to 2-Player Battleships!\n");
    printf("Players take turns attacking each other.\n");

    while (1) {
        printf("\nPlayer %d's Turn:\n", currentPlayer);
        if (currentPlayer == 1) {
            printf("Opponent's Grid:\n");
            displayGrid(player2Grid, 0);
        } else {
            printf("Opponent's Grid:\n");
            displayGrid(player1Grid, 0);
        }

        // Player's turn to attack
        printf("Enter the coordinates to attack (e.g., 1 A): ");
        char colChar;
        scanf("%d %c", &row, &colChar);
        col = colChar - 'A';
        row -= 1;

        if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
            printf("Bro those are invalid coords use your brain.\n");
            continue;
        }

        if (currentPlayer == 1) {
            result = makeMove(player2Grid, row, col);
        } else {
            result = makeMove(player1Grid, row, col);
        }

        if (result == 1) {
            printf("You hit a ship like wow congrats on scrolling above here have some flowers. \n");
        } else if (result == 0) {
            printf("You missed like imagine cant scrolling up its funny \n");
        } else {
            printf("You already attacked this spot. Imagine being soo dumb you cant remember where you attacked earlier.\n");
            continue;
        }

        if (currentPlayer == 1 && isGameOver(player2Grid)) {
            printf("\nCongratulations, Player 1! You destroyed all enemy ships!\n");
            break;
        } else if (currentPlayer == 2 && isGameOver(player1Grid)) {
            printf("\nCongratulations, Player 2! You destroyed all enemy ships!\n");
            break;
        }

        // Switch to the other player
        currentPlayer = (currentPlayer == 1) ? 2 : 1;
    }

    printf("\nFinal Grids:\n");
    printf("Player 1's Grid:\n");
    displayGrid(player1Grid, 1);
    printf("Player 2's Grid:\n");
    displayGrid(player2Grid, 1);

    return 0;
}

void initializeGrid(char grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = '`';
        }
    }
}

void displayGrid(char grid[GRID_SIZE][GRID_SIZE], int revealShips) {
    printf("  ");
    for (char c = 'A'; c < 'A' + GRID_SIZE; c++) {
        printf("%c ", c);
    }
    printf("\n");

    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 'S' && !revealShips) {
                printf("` ");
            } else {
                printf("%c ", grid[i][j]);
            }
        }
        printf("\n");
    }
}

void placeShips(char grid[GRID_SIZE][GRID_SIZE]) {
    int shipsPlaced = 0;
    int row, col;
    char colChar;

    while (shipsPlaced < NUM_SHIPS) {
        printf("Enter ship coordinates lil bro (e.g., 1 A): ");
        scanf("%d %c", &row, &colChar);
        col = colChar - 'A';
        row -= 1;

        if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
            printf("Cant place here lil bro. Place it somewhere in the grid na.\n");
            continue;
        }

        if (grid[row][col] == '`') {
            grid[row][col] = 'S';
            shipsPlaced++;
        } else {
            printf("A ship is placed here dummy. Try again somewhere else you stopid bro.\n");
        }
    }
}

int makeMove(char grid[GRID_SIZE][GRID_SIZE], int row, int col) {
    if (grid[row][col] == 'S') {
        grid[row][col] = 'X';
        return 1; // Hit
    } else if (grid[row][col] == '`') {
        grid[row][col] = '*';
        return 0; // Miss
    }
    return -1; // Already attacked
}

int isGameOver(char grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 'S') {
                return 0; // Ships still remain
            }
        }
    }
    return 1; // All ships are destroyed now go home you code inspector
}
