//System headers
#include <stdio.h>
#include <string.h>

//Bit manipulations
//Bitboard data type
#define U64 unsigned long long

#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

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
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum {white, black, both};
enum {rook, bishop};

/* Castling perms
0001 WKCA
0010 WQCA
0100 BKCA
1000 BQCA
*/

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
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define count_bits(bitboard) __builtin_popcountll(bitboard)

//define piece bitboards (6 white pieces, 6 black pieces)
U64 bitboards[12];
//define occupancy bitboards (white, black, both)
U64 occupancies[3];
int side = 0;
int enpas = e3;
int castle; //castling rights
enum {wk = 1, wq = 2, bk = 4, bq = 8};
//Piece encoding
enum {P, N, B, R, Q, K, p, n, b, r, q, k};
//ASCII pieces
char ascii_pieces[] = "PNBRQKpnbrqk";
//unicode pieces
char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

int char_pieces(char pce)
{
    switch (pce)
    {
        case 'P':
            return P;
        case 'N':
            return N;
        case 'B':
            return B;
        case 'R':
            return R;
        case 'Q':
            return Q;
        case 'K':
            return K;
        case 'p':
            return p;
        case 'n':
            return n;
        case 'b':
            return b;
        case 'r':
            return r;
        case 'q':
            return q;
        case 'k':
            return k;
        default:
            printf("Error in char_pieces");
            return 0;
    }
}

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
unsigned int random_state = 1804289383;
//gen 32bit PSR
unsigned int get_random_number()
{
    unsigned int number = random_state;
    //xor shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    //update state so different next rng query
    random_state = number;

    return number;
}

//generate 64-bit pseudo legal numbers
U64 get_random_U64_number()
{
    U64 n1, n2, n3, n4;
    //Slice each to 16 bit sections
    n1 = (U64)(get_random_number()) & 0xFFFF;
    n2 = (U64)(get_random_number()) & 0xFFFF;
    n3 = (U64)(get_random_number()) & 0xFFFF;
    n4 = (U64)(get_random_number()) & 0xFFFF;

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

void print_board()
{
    printf("\n");
    
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            int piece = -1;

            if (!file)
            {
                printf(" %d ", 8 - rank);
            }

            //Looping through all pieces
            for (int bb_piece = P; bb_piece <= k; bb_piece++)
            {
                if (get_bit(bitboards[bb_piece], square))
                {
                    piece = bb_piece;
                }
            }

            printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
        }
        printf("\n");
    }
    printf("\n    a b c d e f g h\n\n");

    printf("       Side:     %s\n", !side ? "white" : "black");

    printf("       Enpas: %s\n", enpas != no_sq ? square_to_coordinates[enpas] : "nosq");

    printf("       Castling: %c%c%c%c\n\n", (castle & wk) ? 'K' : '-', 
                                            (castle & wq) ? 'Q' : '-', 
                                            (castle & bk) ? 'k' : '-', 
                                            (castle & bq) ? 'q' : '-');
}

//Should be stricter... should we ensure legal position? What if wonky like castling perms are set but no rooks..? Unexpected behavior
void parse_fen(char *fen)
{
    // reset board position (bitboards)
    memset(bitboards, 0ULL, sizeof(bitboards));
    
    // reset occupancies (bitboards)
    memset(occupancies, 0ULL, sizeof(occupancies));
    
    // reset game state variables
    side = 0;
    enpas = no_sq;
    castle = 0;
    
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            // init current square
            int square = rank * 8 + file;
            
            // match ascii pieces within FEN string
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                // init piece type
                int piece = char_pieces(*fen);
                
                // set piece on corresponding bitboard
                set_bit(bitboards[piece], square);
                
                // increment pointer to FEN string
                *fen++;
            }
            
            // match empty square numbers within FEN string
            if (*fen >= '0' && *fen <= '9')
            {
                // init offset (convert char 0 to int 0)
                int offset = *fen - '0';
                
                // define piece variable
                int piece = -1;
                
                // loop over all piece bitboards
                for (int bb_piece = P; bb_piece <= k; bb_piece++)
                {
                    // if there is a piece on current square
                    if (get_bit(bitboards[bb_piece], square))
                        // get piece code
                        piece = bb_piece;
                }
                
                // on empty current square
                if (piece == -1)
                    // decrement file
                    file--;
                
                // adjust file counter
                file += offset;
                
                // increment pointer to FEN string
                *fen++;
            }
            
            // match rank separator
            if (*fen == '/')
                // increment pointer to FEN string
                *fen++;
        }
    }
    
    // got to parsing side to move (increment pointer to FEN string)
    *fen++;
    
    // parse side to move
    (*fen == 'w') ? (side = white) : (side = black);
    
    // go to parsing castling rights
    fen += 2;
    
    // parse castling rights
    while (*fen != ' ')
    {
        switch (*fen)
        {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
            case '-': break;
        }

        // increment pointer to FEN string
        *fen++;
    }
    
    // got to parsing enpassant square (increment pointer to FEN string)
    *fen++;
    
    // parse enpassant square
    if (*fen != '-')
    {
        // parse enpassant file & rank
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');
        
        // init enpassant square
        enpas = rank * 8 + file;
    }
    
    // no enpassant square
    else
        enpas = no_sq;
    
    // loop over white pieces bitboards
    for (int piece = P; piece <= K; piece++)
        // populate white occupancy bitboard
        occupancies[white] |= bitboards[piece];
    
    // loop over black pieces bitboards
    for (int piece = p; piece <= k; piece++)
        // populate white occupancy bitboard
        occupancies[black] |= bitboards[piece];
    
    // init all occupancies
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];
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

U64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL, 
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL, 
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};

U64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL, 
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};

//Pawn attack tables (side, square)
U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];

//Slider piece attack tables. "masks" = storing mask_bishop_attacks()
U64 bishop_masks[64];
U64 rook_masks[64];
U64 bishop_attacks[64][512]; //Squares, Possible occupancies (2 ** maxmoves)
U64 rook_attacks[64][4096]; //Squares, Possible occupancies (2 ** maxmoves)

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

//Magics
U64 find_magic_number(int square, int relevant_bits, int bishop)
{
    //init occupancies (2^12 - maximum possible attack permutations for a given sliding piece)
    U64 occupancies[4096];
    U64 attacks[4096];  //attack table
    U64 used_attacks[4096];

    //init attack mask for a current piece
    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
    //we are initializing occupancies - set of all possible blockers,
    //then all attacks
    //occupancy indicies (highest # index) - remember occupancy is set of all possible "blockers" (occupied squares that we care about)
    int occupancy_indicies = 1 << relevant_bits;
    for (int index = 0; index < occupancy_indicies; index++)
    {
        occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);
        attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index]) : 
                                    rook_attacks_on_the_fly(square, occupancies[index]);
    }

    //test magic numbers loop 
    for (int random_count = 0; random_count < 100000000; random_count++)
    {
        U64 magic_number = generate_magic_number();
        //skip inappropriate magic numbers
        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;
        //init used attacks array
        memset(used_attacks, 0ULL, sizeof(used_attacks));
        //init index & fail_flag
        int index, fail;
        //test magic index
        for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++)
        {
            //serves as a potential index for bitboard attack table
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));
            if (used_attacks[magic_index] == 0ULL) //we are generating indicies 
            {
                //magic index works
                used_attacks[magic_index] = attacks[index];
            }
            else if (used_attacks[magic_index] != attacks[index])
            {
                //magic index doesn't work
                fail = 1;
            }
        }
        if (!fail)
        {
            return magic_number;
        }
    }
    //Never want to get here...
    printf("Magic number fails!");
    return 0ULL;
}

//init magic numbers
void init_magic_numbers()
{
    //loop over 64 board squares
    for (int square = 0; square < 64; square++)
    {
        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);
    }
    for (int square = 0; square < 64; square++)
    {
        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
    }
}

//init slider piece's attack tables
void init_sliders_attacks(int bishop)
{
    //init bishop & rook masks
    for (int square = 0; square < 64; square++)
    {
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        //init current mask
        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];
        //init relevant occupancy bit count
        int relevant_bits_count = count_bits(attack_mask);
        //init occupancy indicies
        int occupancy_indicies = (1 << relevant_bits_count);
        for (int index = 0; index < occupancy_indicies; index++)
        {
            //bishop
            if (bishop)
            {
                //init current occupancy variation
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                //init magic index
                int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
                //init bishop attacks
                bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy); 
            }
            else 
            {
                //init current occupancy variation
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                //init magic index
                int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
                //init bishop attacks
                rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy); 
            }
        }
    }
}

//not a table though... should make a table so no function repetition
//to easily reference table (occupancy is really just a general bitboard)
static inline U64 get_bishop_attacks(int square, U64 occupancy)
{
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];
    return bishop_attacks[square][occupancy];
}

//to easily reference table (occupancy is really just a general bitboard)
static inline U64 get_rook_attacks(int square, U64 occupancy)
{
    occupancy &= rook_masks[square];
    occupancy *= (rook_magic_numbers[square]);
    occupancy >>= 64 - rook_relevant_bits[square];
    return rook_attacks[square][occupancy];
}
static inline U64 get_queen_attacks(int square, U64 occupancy)
{
    U64 queen_attacks = 0ULL;

    U64 bishop_occupancy = occupancy;
    U64 rook_occupancy = occupancy;

    bishop_occupancy &= bishop_masks[square];
    bishop_occupancy *= bishop_magic_numbers[square];
    bishop_occupancy >>= 64 - bishop_relevant_bits[square];

    queen_attacks = bishop_attacks[square][bishop_occupancy];

    rook_occupancy &= rook_masks[square];
    rook_occupancy *= (rook_magic_numbers[square]);
    rook_occupancy >>= 64 - rook_relevant_bits[square];

    queen_attacks |= rook_attacks[square][rook_occupancy];

    return queen_attacks;
}

void init_all() 
{
    init_leaper_attacks();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
}

//Is square attacked given a side?
static inline int is_square_attacked(int square, int side)
{
    //If there is a white pawn in the bottom left / bottom right sqs
    if ((side == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;
    if ((side == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;
    
    //If knights are on any of the knight attacking squares
    if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;
    if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

    if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;
    if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;
    if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return 1;
    return 0;
}

void print_attacked_squares(int side)
{
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            if (!file)
            {
                printf("  %d ", 8 - rank);
            }
            printf("%d ", is_square_attacked(square, side) ? 1 : 0);
        }
        printf("\n");
    }
    printf("    a b c d e f g h\n\n");
}


/*
          binary move bits                               hexidecimal constants
    
    0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0001 0000 0000 0000 0000 0000    capture flag        0x100000
    0010 0000 0000 0000 0000 0000    double push flag    0x200000
    0100 0000 0000 0000 0000 0000    enpassant flag      0x400000
    1000 0000 0000 0000 0000 0000    castling flag       0x800000
*/

// encode move
#define encode_move(source, target, piece, promoted, capture, double, enpas, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (double << 21) |    \
    (enpas << 22) | \
    (castling << 23)    \
    
// extract source square
#define get_move_source(move) (move & 0x3f)

// extract target square
#define get_move_target(move) ((move & 0xfc0) >> 6)

// extract piece
#define get_move_piece(move) ((move & 0xf000) >> 12)

// extract promoted piece
#define get_move_promoted(move) ((move & 0xf0000) >> 16)

// extract capture flag
#define get_move_capture(move) (move & 0x100000)

// extract double pawn push flag
#define get_move_double(move) (move & 0x200000)

// extract enpassant flag
#define get_move_enpas(move) (move & 0x400000)

// extract castling flag
#define get_move_castling(move) (move & 0x800000)


// move list structure
typedef struct {
    // moves
    int moves[256];
    
    // move count
    int count;
} moves;

// add move to the move list
static inline void add_move(moves *move_list, int move)
{
    // strore move
    move_list->moves[move_list->count] = move;
    
    // increment move count
    move_list->count++;
}

// print move (for UCI purposes)
void print_move(int move)
{
    printf("%s%s\n", square_to_coordinates[get_move_source(move)],
                     square_to_coordinates[get_move_target(move)]);
}


// print move list
void print_move_list(moves *move_list)
{
    if (!move_list->count)
    {
        printf("\nNo moves in the move list!\n");
        return;
    }
    
    printf("\n    move    piece   capture   double    enpass    castling\n\n");
    
    // loop over moves within a move list
    for (int move_count = 0; move_count < move_list->count; move_count++)
    {
        // init move
        int move = move_list->moves[move_count];
        
        #ifdef WIN64
            // print move
            printf("    %s%s    %c       %d         %d         %d         %d\n", square_to_coordinates[get_move_source(move)],
                                                                                  square_to_coordinates[get_move_target(move)],
                                                                                  ascii_pieces[get_move_piece(move)],
                                                                                  get_move_capture(move) ? 1 : 0,
                                                                                  get_move_double(move) ? 1 : 0,
                                                                                  get_move_enpas(move) ? 1 : 0,
                                                                                  get_move_castling(move) ? 1 : 0);
        #else
            // print move
            printf("    %s%s    %s       %d         %d         %d         %d\n", square_to_coordinates[get_move_source(move)],
                                                                                  square_to_coordinates[get_move_target(move)],
                                                                                  unicode_pieces[get_move_piece(move)],
                                                                                  get_move_capture(move) ? 1 : 0,
                                                                                  get_move_double(move) ? 1 : 0,
                                                                                  get_move_enpas(move) ? 1 : 0,
                                                                                  get_move_castling(move) ? 1 : 0);
        #endif
    }
    // print total number of moves
    printf("\n\n    Total number of moves: %d\n\n", move_list->count);
}


static inline void generate_moves(moves *move_list)
{   
    move_list->count = 0;
    
    //(pce current position, pce future position)
    int source_square, target_square;
    //define current piece's bitboard copy & its attacks
    U64 bitboard, attacks;
    //Idea: loop through all piece bitboards, loop through all bits on those using ls1b, generate moves

    for (int piece = P; piece <= k; piece++)
    {
        bitboard = bitboards[piece];
        //Pawns & castling moves
        if (side == white)
        {
            if (piece == P)
            {
                while (bitboard) //loop until no more P
                {
                    source_square = get_ls1b_index(bitboard);
                    target_square = source_square - 8;

                    //generate quiet pawn moves
                    if (!(target_square < a8) && !get_bit(occupancies[both], target_square))
                    {
                        //pawn promotion... can we use masks instead?
                        if (source_square >= a7 && source_square <= h7)
                        {
                            //add move into move list
                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
                        }
                        else 
                        {
                            //one ahead
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            //two ahead
                            //i feel like this could be sped up using bitwise operations
                            if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
                            {
                                //add move into move list
                                add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
                            }
                        }
                    }

                    //attack pawn moves
                    attacks = pawn_attacks[white][source_square] & occupancies[black];

                    while (attacks)
                    {
                        target_square = get_ls1b_index(attacks);

                        //pawn promotion... can we use masks instead?
                        if (source_square >= a7 && source_square <= h7)
                        {
                            //add move into move list
                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
                        }
                        else 
                        {
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }

                        pop_bit(attacks, target_square);
                    }

                    //enpas
                    if (enpas != no_sq)
                    {
                        U64 enpas_attacks = pawn_attacks[side][source_square] & (1ULL << enpas);
                        if (enpas_attacks)
                        {
                            int target_enpas = get_ls1b_index(enpas_attacks);
                            add_move(move_list, encode_move(source_square, target_enpas, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    //pop ls1b from bb copy
                    pop_bit(bitboard, source_square);
                }        
            }
            //castling case... squares between need to be empty, not in check. rooks can't have moved
            else if (piece == K)
            {
                //WKCA
                if (castle & wk)
                {
                    if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
                    {
                        //why don't we check castling square..?
                        if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                        {
                            add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
                //WQCA
                if (castle & wq)
                {
                    if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
                    {
                        //why don't we check castling square..?
                        if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                        {
                            add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }
        }
        else 
        {
            if (piece == p)
            {
                while (bitboard) //loop until no more P
                {
                    source_square = get_ls1b_index(bitboard);
                    target_square = source_square + 8;

                    //generate quiet pawn moves
                    if (!(target_square > h1) && !get_bit(occupancies[both], target_square))
                    {
                        //pawn promotion... can we use masks instead?
                        if (source_square >= a2 && source_square <= h2)
                        {
                            //add move into move list
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
                        }
                        else 
                        {
                            //one ahead
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            //two ahead
                            //i feel like this could be sped up using bitwise operations
                            if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
                            {
                                //add move into move list
                                add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
                            }
                        }
                    }

                    //attack pawn moves
                    attacks = pawn_attacks[black][source_square] & occupancies[white];

                    while (attacks)
                    {
                        target_square = get_ls1b_index(attacks);

                        //pawn promotion... can we use masks instead?
                        if (source_square >= a2 && source_square <= h2)
                        {
                            //add move into move list
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
                        }
                        else 
                        {
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }

                        pop_bit(attacks, target_square);
                    }

                    //enpas
                    if (enpas != no_sq)
                    {
                        U64 enpas_attacks = pawn_attacks[side][source_square] & (1ULL << enpas);
                        if (enpas_attacks)
                        {
                            int target_enpas = get_ls1b_index(enpas_attacks);
                            add_move(move_list, encode_move(source_square, target_enpas, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    //pop ls1b from bb copy
                    pop_bit(bitboard, source_square);
                }        
            }
            //castling case... squares between need to be empty, not in check. rooks can't have moved
            else if (piece == k)
            {
                //BKCA
                if (castle & bk)
                {
                    if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
                    {
                        //why don't we check castling square..?
                        if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                        {
                            add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
                //BQCA
                if (castle & bq)
                {
                    if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
                    {
                        //why don't we check castling square..? (handled in movegen)
                        if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                        {
                            add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }
        }
        //All other moves
        if ((side == white) ? piece == N : piece == n)
        {
            while (bitboard)
            {
                source_square = get_ls1b_index(bitboard);
                
                attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks)
                {
                    target_square = get_ls1b_index(attacks);
                    //quiet
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else 
                    {
                        printf("piece capture: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    //capture move
                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        }

        if ((side == white) ? piece == B : piece == b)
        {
            while (bitboard)
            {
                source_square = get_ls1b_index(bitboard);
                //do we need that last part?
                attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks)
                {
                    target_square = get_ls1b_index(attacks);
                    //quiet
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else 
                    {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    //capture move
                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        }

        if ((side == white) ? piece == R : piece == r)
        {
            while (bitboard)
            {
                source_square = get_ls1b_index(bitboard);
                
                attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks)
                {
                    target_square = get_ls1b_index(attacks);
                    //quiet
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else 
                    {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    //capture move
                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        }

        if ((side == white) ? piece == Q : piece == q)
        {
            while (bitboard)
            {
                source_square = get_ls1b_index(bitboard);
                
                attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks)
                {
                    target_square = get_ls1b_index(attacks);
                    //quiet
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else 
                    {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    //capture move
                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        }

        if ((side == white) ? piece == K : piece == k)
        {
            while (bitboard)
            {
                source_square = get_ls1b_index(bitboard);
                
                attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                while (attacks)
                {
                    target_square = get_ls1b_index(attacks);
                    //quiet
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else 
                    {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    //capture move
                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        }
    }
}


// preserve board state
#define copy_board()                                                      \
    U64 bitboards_copy[12], occupancies_copy[3];                          \
    int side_copy, enpas_copy, castle_copy;                               \
    memcpy(bitboards_copy, bitboards, 96);                                \
    memcpy(occupancies_copy, occupancies, 24);                            \
    side_copy = side, enpas_copy = enpas, castle_copy = castle;           \

// restore board state
#define take_back()                                                       \
    memcpy(bitboards, bitboards_copy, 96);                                \
    memcpy(occupancies, occupancies_copy, 24);                            \
    side = side_copy, enpas = enpas_copy, castle = castle_copy;           \


int main()
{
    // init all
    init_all();
    
    // parse fen
    parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq c6 0 1 ");
    print_board();
    
    // preserve board state
    copy_board();
    
    // parse fen
    parse_fen(empty_board);
    print_board();
    
    // restore board state
    take_back();

    print_board();
    
    return 0;
}