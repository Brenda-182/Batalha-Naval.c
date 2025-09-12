#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 10
#define SHIP_SIZE  3
#define WATER      0
#define SHIP_MARK  3
#define AOE_MARK   5

// -------------------- Utilidades de tabuleiro --------------------
void initBoard(int b[BOARD_SIZE][BOARD_SIZE]) {
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            b[r][c] = WATER;
}

void initOverlay(int o[BOARD_SIZE][BOARD_SIZE]) {
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            o[r][c] = 0;
}

// Combinando tabuleiro e overlay 
void printBoardWithOverlay(int board[BOARD_SIZE][BOARD_SIZE],
                           int overlay[BOARD_SIZE][BOARD_SIZE]) {
    printf("    ");
    for (int c = 0; c < BOARD_SIZE; c++) printf("%d ", c);
    printf("\n    ");
    for (int c = 0; c < BOARD_SIZE; c++) printf("--");
    printf("\n");

    for (int r = 0; r < BOARD_SIZE; r++) {
        printf("%2d | ", r);
        for (int c = 0; c < BOARD_SIZE; c++) {
            int cell = (overlay[r][c] == AOE_MARK) ? AOE_MARK : board[r][c];
            printf("%d ", cell);
        }
        printf("\n");
    }
    printf("\n");
}

// -------------------- Posicionamento de navios --------------------
int canPlace(int b[BOARD_SIZE][BOARD_SIZE], int r, int c, int dr, int dc) {
    for (int k = 0; k < SHIP_SIZE; k++) {
        int nr = r + dr * k, nc = c + dc * k;
        if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE) return 0;
        if (b[nr][nc] != WATER) return 0;
    }
    return 1;
}
void place(int b[BOARD_SIZE][BOARD_SIZE], int r, int c, int dr, int dc) {
    for (int k = 0; k < SHIP_SIZE; k++)
        b[r + dr * k][c + dc * k] = SHIP_MARK;
}
int placeH(int b[BOARD_SIZE][BOARD_SIZE], int r, int c) { return canPlace(b,r,c,0,1) ? (place(b,r,c,0,1),1) : 0; }
int placeV(int b[BOARD_SIZE][BOARD_SIZE], int r, int c) { return canPlace(b,r,c,1,0) ? (place(b,r,c,1,0),1) : 0; }
int placeD1(int b[BOARD_SIZE][BOARD_SIZE], int r, int c){ return canPlace(b,r,c,1,1) ? (place(b,r,c,1,1),1) : 0; }
int placeD2(int b[BOARD_SIZE][BOARD_SIZE], int r, int c){ return canPlace(b,r,c,1,-1)? (place(b,r,c,1,-1),1): 0; }

// -------------------- Matrizes de habilidades  --------------------
// Preenche mat[N][N] com 0/1 usando condicionais em loops aninhados.

// Cone (aponta para baixo): ápice na linha 0, expandindo até o meio.
// Centro colunar = N/2; célula é 1 se |col - centro| <= linha.
void buildCone(int N, int mat[N][N]) {
    int centro = N / 2;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            int dentro = (abs(c - centro) <= r) && (r <= centro);
            mat[r][c] = dentro ? 1 : 0;
        }
    }
}

// Cruz (em mais): linha central e coluna central = 1.
void buildCross(int N, int mat[N][N]) {
    int centro = N / 2;
    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            mat[r][c] = (r == centro || c == centro) ? 1 : 0;
}

// Octaedro (losango na visão frontal): |dr| + |dc| <= raio.
void buildOctahedron(int N, int mat[N][N]) {
    int centro = N / 2, raio = centro;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            int dist = abs(r - centro) + abs(c - centro);
            mat[r][c] = (dist <= raio) ? 1 : 0;
        }
    }
}

// -------------------- Sobreposição no tabuleiro --------------------
// Aplica mat[N][N] no overlay, centrando em (or, oc)
void applyAbilityCentered(int overlay[BOARD_SIZE][BOARD_SIZE],
                          int N, int mat[N][N],
                          int or, int oc) {
    int centro = N / 2;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            if (mat[r][c] != 1) continue;
            int br = or + (r - centro);
            int bc = oc + (c - centro);
            if (br >= 0 && br < BOARD_SIZE && bc >= 0 && bc < BOARD_SIZE) {
                overlay[br][bc] = AOE_MARK;
            }
        }
    }
}

int main(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int overlay[BOARD_SIZE][BOARD_SIZE];
    initBoard(board);
    initOverlay(overlay);

    // --------- Posiciona 4 navios (2 ortogonais + 2 diagonais) ---------
    // Ajuste mantendo limites e sem sobrepor.
    if (!placeH(board, 2, 4)) { printf("Falha H\n"); return 1; }
    if (!placeV(board, 6, 1)) { printf("Falha V\n"); return 1; }
    if (!placeD1(board, 0, 0)) { printf("Falha D1\n"); return 1; }
    if (!placeD2(board, 0, 9)) { printf("Falha D2\n"); return 1; }

    // --------- Matrizes de habilidades ---------
    const int N = 5;
    int cone[5][5], cross[5][5], octa[5][5];
    buildCone(N, cone);
    buildCross(N, cross);
    buildOctahedron(N, octa);

    // --------- Define origens no tabuleiro ---------
    // Use valores entre 2..7 para N=5 garantir encaixe no 10x10 sem cortar.
    int origemConeR = 3, origemConeC = 3;    
    int origemCrossR = 5, origemCrossC = 7;
    int origemOctaR = 7, origemOctaC = 4;

    // --------- Aplica habilidades ---------
    applyAbilityCentered(overlay, N, cone,  origemConeR,  origemConeC);
    applyAbilityCentered(overlay, N, cross, origemCrossR, origemCrossC);
    applyAbilityCentered(overlay, N, octa,  origemOctaR,  origemOctaC);

    // --------- Exibe resultado ---------
    printf("Legenda: 0=agua, 3=navio, 5=area de habilidade\n\n");
    printBoardWithOverlay(board, overlay);

    return 0;
}
