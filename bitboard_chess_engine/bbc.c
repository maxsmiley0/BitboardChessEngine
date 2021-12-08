//System headers
#include <stdio.h>

//Bit manipulations
//Bitboard data type
#define U64 unsigned long long

//So cool!! We can actually generate massive data in code by printing it formatted like that
//board squares
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,      
    a7, b7, c7, d7, e7, f7, g7, h7,      
    a6, b6, c6, d6, e6, f6, g6, h6,      
    a5, b5, c5, d5, e5, f5, g5, h5,      
    a4, b4, c4, d4, e4, f4, g4, h4,      
    a3, b3, c3, d3, e3, f3, g3, h3,      
    a2, b2, c2, d2, e2, f2, g2, h2,      
    a1, b1, c1, d1, e1, f1, g1, h1
};

enum {white, black};

/*
"a8", "b8", "c8", "e8", "f8", "g8", "h8",
"a7", "b7", "c7", "e7", "f7", "g7", "h7",
"a6", "b6", "c6", "e6", "f6", "g6", "h6",
"a5", "b5", "c5", "e5", "f5", "g5", "h5",
"a4", "b4", "c4", "e4", "f4", "g4", "h4",
"a3", "b3", "c3", "e3", "f3", "g3", "h3",
"a2", "b2", "c2", "e2", "f2", "g2", "h2",
"a1", "b1", "c1", "e1", "f1", "g1", "h1",
*/

//set/get/pop macros
//gets nth bit of square
#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
//deletes bit... why don't we just & with 0?
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0)

void print_bitboard(U64 bitboard)
{
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            //Convert file & rank into square index
            int square = rank * 8 + file;

            //print ranks
            if (!file)
            {
                printf("  %d  ", 8 - rank);
            }

            //shifts left to get the nth bit value, prints 1 if hits 0 if misses
            printf(" %d ", get_bit(bitboard, square) ? 1 : 0);
        }
        printf("\n");
    }

    printf("\n      a  b  c  d  e  f  g  h\n\n");
    printf("      Bitboard: %llud\n\n", bitboard);
}

const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 not_hg_file = 4557430888798830399ULL;
const U64 not_ab_file = 18229723555195321596ULL;

//Pawn attack tables (side, square)
U64 pawn_attacks[2][64];

//generate pawn attacks
U64 mask_pawn_attacks(int side, int square)
{
    //result attacs bitboard
    U64 attacks = 0ULL;
    
    //piece bitboard
    U64 bitboard = 0ULL;

    //set piece on board
    set_bit(bitboard, square);
    //white pawns
    if (!side)
    {
        //To prevent out of bounds shenanigans
        if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    }

    //black pawns
    else 
    {
        //To prevent out of bounds shenanigans
        if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
        if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    }

    //return attack map
    return attacks;
}

//init leaper pieces attacks
void init_leaper_attacks()
{
    //loop over 64 board square
    for (int square = 0; square < 64; square++)
    {
        //init pawn attacks
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    }
}
//Main driver
int main()
{
    init_leaper_attacks();
    for (int square = 0; square < 64; square++)
    {
        print_bitboard(pawn_attacks[white][square]);
        print_bitboard(pawn_attacks[black][square]);
    }
    return 0;
}