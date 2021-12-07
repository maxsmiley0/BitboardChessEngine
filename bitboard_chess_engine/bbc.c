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

//Main driver
int main()
{
    U64 bitboard = 0ULL;

    set_bit(bitboard, e4);
    set_bit(bitboard, c3);
    set_bit(bitboard, f2);
    pop_bit(bitboard, e4);
    pop_bit(bitboard, e4);
    print_bitboard(bitboard);

    return 0;
}