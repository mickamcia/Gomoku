#include <stdio.h>
#include <string.h>
#define U64 unsigned long long
#define U8 unsigned char

#define macro_set_bit(board, index, color) ((board).bits[color] |= (1ULL <<(U64)(index)))
#define macro_get_bit(board, index, color) ((board).bits[color] & (1ULL <<(U64)(index)))
#define macro_pop_bit(board, index, color) ((board).bits[color] &= ~(1ULL << (U64)(index)))

#define file0 ~0x8080808080808080ULL
#define file7 ~0x0101010101010101ULL
#define FACTOR 10

typedef struct Board{
    U64 bits[2];
    U8 move;
} Board;

enum {white = 0, black = 1, empty = 2};
char* unicode[] = {[0] = "⛂ ", [1] = "⛀ ", [2] = "・"};
char* coords[] = {"０","１","２","３","４","５","６","７"};

const U64 mask5[] = 
{
    //horizontal
    0x1f, 0x3e, 0x7c, 0xf8, 
    0x1f00, 0x3e00, 0x7c00, 0xf800, 
    0x1f0000, 0x3e0000, 0x7c0000, 0xf80000, 
    0x1f000000, 0x3e000000, 0x7c000000, 0xf8000000, 
    0x1f00000000, 0x3e00000000, 0x7c00000000, 0xf800000000, 
    0x1f0000000000, 0x3e0000000000, 0x7c0000000000, 0xf80000000000, 
    0x1f000000000000, 0x3e000000000000, 0x7c000000000000, 0xf8000000000000, 
    0x1f00000000000000, 0x3e00000000000000, 0x7c00000000000000, 0xf800000000000000,

    //vertical
    0x101010101, 0x10101010100, 0x1010101010000, 0x101010101000000, 
    0x202020202, 0x20202020200, 0x2020202020000, 0x202020202000000, 
    0x404040404, 0x40404040400, 0x4040404040000, 0x404040404000000, 
    0x808080808, 0x80808080800, 0x8080808080000, 0x808080808000000, 
    0x1010101010, 0x101010101000, 0x10101010100000, 0x1010101010000000, 
    0x2020202020, 0x202020202000, 0x20202020200000, 0x2020202020000000, 
    0x4040404040, 0x404040404000, 0x40404040400000, 0x4040404040000000, 
    0x8080808080, 0x808080808000, 0x80808080800000, 0x8080808080000000, 

    //upleft-downright
    0x1008040201, 0x2010080402, 0x4020100804, 0x8040201008,
    0x100804020100, 0x201008040200, 0x402010080400, 0x804020100800, 
    0x10080402010000, 0x20100804020000, 0x40201008040000, 0x80402010080000, 
    0x1008040201000000, 0x2010080402000000, 0x4020100804000000, 0x8040201008000000,

    //upright-downleft
    0x102040810, 0x204081020, 0x408102040, 0x810204080,
    0x10204081000, 0x20408102000, 0x40810204000, 0x81020408000, 
    0x1020408100000, 0x2040810200000, 0x4081020400000, 0x8102040800000, 
    0x102040810000000, 0x204081020000000, 0x408102040000000, 0x810204080000000,
};
static inline int get_population_count(U64 bitboard)
{
    int count = 0;
    while (bitboard)
    {
        count++;
        bitboard &= bitboard - 1;
    }
    return count;
}

static inline int get_least_bit_index(U64 bitboard)
{
    if (bitboard)
    {
        return get_population_count((bitboard & -bitboard) - 1);
    }
    else
        return -1;
}

void print_position(Board board){
    printf("\n  ");
    for(int i = 0; i < 8; i++){
        printf("%s", coords[i]);
    }
    for(int i = 0; i < 8; i++){
        printf("\n%s", coords[i]);
        for(int j = 0; j < 8; j++){
            if(macro_get_bit(board, i * 8 + j, white)) printf("%s", unicode[white]);
            else if(macro_get_bit(board, i * 8 + j, black)) printf("%s", unicode[black]);
            else printf("%s", unicode[empty]);
        }
    }
    printf("\n\nSide: %s\n", board.move & 1 ? "black" : "white");
}
void get_user_input(Board* board, int* input){
    char row, col;
    while(1){
        *input = 0;
        row = col = -1;
        scanf("\n\n%c%c",&row, &col);
        *input += (row - '0') * 8;
        *input += (col - '0');
        if((*input) < 0 || (*input) >= 64) printf("invalid coordinates\n");
        else if(macro_get_bit(*board, *input, white) || macro_get_bit(*board, *input, black)) printf("can not override\n");
        else break;
    }
    printf("\ninput: %c%c\n", row, col);
}
int evaluate_board(Board* board){
    int score = 0;
    int ally = (board->move & 1) ? black : white;
    int enemy = (board->move & 1) ? white : black;
    for(int i = 0; i < 96; i++){
        if((board->bits[ally] & mask5[i]) == mask5[i]) score += 1;
        if((board->bits[enemy] & mask5[i]) == mask5[i]) score -= 1 * FACTOR;
    }
    return score;
}
static inline U64 move_gen(Board* board){
    U64 bits = board->bits[0] | board->bits[1];
    return ((bits << 8) | (bits >> 8) | ((bits & file0) << 1) | ((bits & file7) >> 1) | ((bits & file0) << 9) | ((bits & file7) >> 9) | ((bits & file7) << 7) | ((bits & file0) >> 7)) & (~bits);
}
int negamax(Board* board, int alpha, int beta, int depth, int depth_start, int* input, int* nodes){ 
    U64 moves = move_gen(board); 
    int move_count = get_population_count(moves);
    if(depth <= 0){
        (*nodes)++;
        return evaluate_board(board);
    }
    for(int i = 0; i < move_count; i++){
        int curr = get_least_bit_index(moves);
        moves ^= (1ULL << curr);
        macro_set_bit(*board, curr, (board->move & 1) ? black : white);
        board->move++;
        int score = -negamax(board, -beta, -alpha, depth - 1, depth_start, input, nodes);
        macro_pop_bit(*board, curr, (board->move & 1) ? white : black);
        board->move--;
        if(score > beta) return beta;
        if(score > alpha){
            alpha = score;
            if(depth == depth_start) *input = curr;
        }
    }
    return alpha;
}
void get_comp_input(Board* board, int* input){
    int nodes = 0, score;
    int depth = 1;
    while(nodes < 0xffff && depth <= get_population_count(~(board->bits[0] | board->bits[1]))){
        nodes = 0;
        score = -negamax(board, -0xffff, 0xffff, depth, depth, input, &nodes);
        printf("Score: %d Depth: %d Nodes: %d\n", score, depth, nodes);
        depth++;
    }
}
int main(){
    Board board;
    int input;
    memset(&board, 0, sizeof(Board));
    printf("'w' if you want to start");
    char c;
    scanf("%c",&c);
    if(c != 'w'){
        macro_set_bit(board, 28, (board.move & 1) ? black : white);
        board.move++;
    }
    while(~(board.bits[0] | board.bits[1])){
        print_position(board);
        printf("Score: %d\n", evaluate_board(&board));

        get_user_input(&board, &input);
        macro_set_bit(board, input, (board.move & 1) ? black : white);
        board.move++;

        get_comp_input(&board, &input);
        macro_set_bit(board, input, (board.move & 1) ? black : white);
        board.move++;
    }
    print_position(board);
    return 0;
}