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

const char* square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

//set/get/pop macros
//gets nth bit of square
#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
//deletes bit... why don't we just & with 0?
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0)
#define count_bits(bitboard) __builtin_popcountll(bitboard)

//rng
//for ls1b de bruijn multiplication
const int index64[64] = {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};

//PSRN state
unsigned int state = 1804289383;
//gen 32bit PSR
unsigned int get_random_number()
{
    unsigned int number = state;
    //xor shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    //update state so different next rng query
    state = number;

    return number;
}

//generate 64-bit pseudo legal numbers
U64 get_random_U64_number()
{
    U64 n1, n2, n3, n4;
    //Slice each to 16 bit sections
    n1 = (U64)(get_random_number() & 0xFFFF);
    n2 = (U64)(get_random_number() & 0xFFFF);
    n3 = (U64)(get_random_number() & 0xFFFF);
    n4 = (U64)(get_random_number() & 0xFFFF);

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

//candidate... 1/8 populated
U64 generate_magic_number()
{
    return (get_random_U64_number() & get_random_U64_number() & get_random_U64_number());
}

static inline int get_ls1b_index(U64 bitboard) {
   const U64 debruijn64 = 0x03f79d71b4cb0a89;
   if (bitboard)
   {
       return index64[((bitboard & -bitboard) * debruijn64) >> 58];
   }
   else 
   {
       //return illegal index
       return -1;
   }
}

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

//"occupancy bit count for every square of the board"
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,         
    5, 5, 5, 5, 5, 5, 5, 5,        
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

//Pawn attack tables (side, square)
U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];

//generate pawn attacks
U64 mask_pawn_attacks(int side, int square)
{
    //result attacks bitboard
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
//generate knight attacks
U64 mask_knight_attacks(int square)
{
    //result attacks bitboard
    U64 attacks = 0ULL;
    
    //piece bitboard
    U64 bitboard = 0ULL;

    //set piece on board
    set_bit(bitboard, square);
    
    //Don't need to guard up or down cause gets completely shifted out of bb..?
    if ((bitboard >> 17) & not_h_file) attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & not_a_file) attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & not_hg_file) attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & not_ab_file) attacks |= (bitboard >> 6);

    if ((bitboard << 17) & not_a_file) attacks |= (bitboard << 17);
    if ((bitboard << 15) & not_h_file) attacks |= (bitboard << 15);
    if ((bitboard << 10) & not_ab_file) attacks |= (bitboard << 10);
    if ((bitboard << 6) & not_hg_file) attacks |= (bitboard << 6);

    //return attack map
    return attacks;
}
//generate king attacks
U64 mask_king_attacks(int square)
{
    //result attacks bitboard
    U64 attacks = 0ULL;
    
    //piece bitboard
    U64 bitboard = 0ULL;

    //set piece on board
    set_bit(bitboard, square);
    
    //Don't need to guard up or down cause gets completely shifted out of bb..?
    if (bitboard >> 8) attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);

    if (bitboard << 8) attacks |= (bitboard << 8);
    if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
    if ((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);

    //return attack map
    return attacks;
}

//these are all the squares that CAN impact your bishop/rook's mobility when attacking (potential blockers)
//mask bishop attacks
U64 mask_bishop_attacks(int square)
{
    //result attacks bitboard
    U64 attacks = 0ULL;
    
    //init ranks & files
    int r, f;

    //init target rank & files (next target square within ray of slider piece)
    int tr = square / 8;
    int tf = square % 8;

    //mask relevant bishop occupancy bits... why don't we add edges?
    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + f));
    } 
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + f));
    } 
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + f));
    } 
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + f));
    } 

    return attacks;
}
//mask rook attacks
U64 mask_rook_attacks(int square)
{
    //result attacks bitboard
    U64 attacks = 0ULL;
    
    //init ranks & files
    int r, f;

    //init target rank & files (next target square within ray of slider piece)
    int tr = square / 8;
    int tf = square % 8;

    //mask relevant rook occupancy bits... why don't we add edges?
    for (r = tr + 1; r <= 6; r++)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + tf));
    }
    for (r = tr - 1; r >= 1; r--)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + tf));
    }
    for (f = tf + 1; f <= 6; f++)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (tr * 8 + f));
    }
    for (f = tf - 1; f >= 1; f--)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (tr * 8 + f));
    }

    return attacks;
}
//init leaper pieces attacks

//all the squares that the bishop/rook CAN move to unimpeded
//generate bishop attacks on the fly
U64 bishop_attacks_on_the_fly(int square, U64 block)
{
    //generate bishop attacks
    U64 attacks = 0ULL;
    
    //init ranks & files
    int r, f;

    //init target rank & files (next target square within ray of slider piece)
    int tr = square / 8;
    int tf = square % 8;

    //mask relevant bishop occupancy bits... why don't we add edges?
    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    } 
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    } 
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    } 
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    } 

    return attacks;
}
//generate rook attacks on the fly
U64 rook_attacks_on_the_fly(int square, U64 block)
{
    //generate rook attacks
    U64 attacks = 0ULL;
    
    //init ranks & files
    int r, f;

    //init target rank & files (next target square within ray of slider piece)
    int tr = square / 8;
    int tf = square % 8;

    //mask relevant rook occupancy bits... why don't we add edges?
    for (r = tr + 1; r <= 7; r++)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    for (r = tr - 1; r >= 0; r--)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    for (f = tf + 1; f <= 7; f++)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    for (f = tf - 1; f >= 0; f--)
    {
        //reconverting (r,f) -> sq
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }

    return attacks;
}
void init_leaper_attacks()
{
    //loop over 64 board square
    for (int square = 0; square < 64; square++)
    {
        //init pawn attacks
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);

        //init knight attacks
        knight_attacks[square] = mask_knight_attacks(square);

        ///init king attacks
        king_attacks[square] = mask_king_attacks(square);
    }
}

//Magic numbers are index to attack tables that map block boards to attack boards? E.g. many block boards map to same attacks..?

//set occupancies
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask)
{
    // occupancy map
    U64 occupancy = 0ULL;

    //loop over the range of bits within attack mask
    for (int count = 0; count < bits_in_mask; count++)
    {
        //get ls1b of attack mask
        int square = get_ls1b_index(attack_mask);

        //pop ls1b in attack map
        pop_bit(attack_mask, square);

        //make sure occupancy is on board
        if (index & (1 << count))
        {
            //popuate occupancy map
            occupancy |= (1ULL << square);
        }
    }

    return occupancy;   
}

//Main driver
int main()
{
    //init leaper pieces attacks
    init_leaper_attacks();
    print_bitboard(generate_magic_number());
    return 0;
}