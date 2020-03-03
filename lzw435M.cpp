/*  Algorithms: Project 3 Part 2 - Modified LZW Compression
    Brighid Harris( bah128 / 3154693 )
    11.19.2019

    This program will take two arguments, the action the user
    wants to take and the name of the file in which the the action
    will be done to.

    ./lzw435M.o c [ filename ]: 
        Will compress the file with modified LZW compression.
    ./lzw435M.o e [ filename ]:
        Will decompress/expand a file that has been compressed
        using modified LZW compression.
*/

#include <string>
#include <map>
#include <iostream>
#include <fstream>

void compressMod( std::string filename );
void decompressMod( std::string filename );

int main(int argc, char *argv[])
{
    if( argc != 3 )
    {
        std::cout << "Invalid argument, cannot open file or determine action\n";
        return 0;
    }

    std::string test = argv[1];
    std::string file = argv[2];
    if( test == "c" )
        compressMod( file );
    else if( test == "e" )
        decompressMod( file );
    
    std::cout << "done\n";
    return 0;
}

// Converts the binary stirng( p ) to an int
int binary2Int( std::string p )
{
    int code = 0;
    for( auto i : p )
        code = code << 1 | ( i - '0' );
    return code;
}

// Converts the int( code ) into a binary string of size codeLength
std::string int2Binary( int code, int codeLength )
{
    std::string binary( codeLength, '0' );
    for( auto i = ( binary.end() - 1 ); i != ( binary.begin() - 1 ); --i, code >>= 1 )
    {
        if( code % 2 )
            *i = '1';
    }

    if( code > 0 )
        std::cout << "\nWarning: Overflow. Num is too big to be coded by " << codeLength << " bits!\n";
    return binary;
}

// Reads the contents of the file into the 'full' string
void readFile( std::string filename, std::string & full )
{
    std::ifstream file( filename.c_str(), std::ios::binary | std::ios::ate );
    auto size = file.tellg();
    file.seekg(0);
    full.resize( size );
    file.read( &full[0], size );
    file.close();
}

// Compresses a file using modified LZW
void compressMod( std::string filename )
{
    std::cout << "Compressing using modified lzw...\n";
    // Read File
    std::string full;
    readFile( filename, full );

    // initalize the dictionary
    int dictSize = 256;
    std::map< std::string, int > dict;
    for( int i = 0; i < dictSize; ++i )
        dict[ std::string( 1, i ) ] = i;

    int bitSize = 9, limit = dictSize * 2;
    std::string working = "", toVec = "", toFile = "";
    for( auto i = full.begin(); i != full.end(); ++i )
    {
        working += (char) *i;
        if( dict.count( working ) )
            toVec += *i;
        else
        {
            if( dictSize > limit )
            {
                limit <<= 1; 
                ++bitSize;
            }
            if( dictSize < 65535 )
                dict[ working ] = dictSize++;
            toFile += int2Binary( dict[ toVec ], bitSize );
            working = toVec = *i;
        }
    }
    if( !toVec.empty() )
        toFile += int2Binary( dict[ toVec ], bitSize );

    // Pad zeros
    if( toFile.size() % 8 != 0 )
        toFile += ( ( std::string ) "00000000" ).substr( 0, 8 - toFile.size() % 8 );

    // Write to the output File
    std::ofstream outFile( ( filename += ".lzw2" ).c_str(), std::ios::binary );
    for( int i = 0; i < toFile.size(); i += 8 )
        outFile << ( char ) ( binary2Int( toFile.substr( i, 8 )) & 255 );
    outFile.close();
}

// Decompresses a file that was compressed using modified LZW
void decompressMod( std::string filename)
{    
    // Checks to make sure the file that is being decompressed has
    // been compressed using modified lzw compression 
    if( filename.substr( filename.find_last_of('.') ) != ".lzw2" )
    {
        std::cout << filename << " was not compressed using modified LZW\n";
        return;
    }
    std::cout << "Decompressing modified lzw file...\n";
    // Read file
    std::string full;
    readFile( filename, full );

    // Initalize the dictionary
    int dictSize = 256;
    std::map< int, std::string > dict;
    for( int i = 0; i < dictSize; ++i )
        dict[ i ] = std::string( 1, i );

    /*  Set inital values for all variables, decompresses first letter
        bitsNeeded: how many bits are needed from the next compressed char
            in order to get the int value where the next char/word 
            is stored in the dictionary
        shift: how many places we need to shift to the left in order to get
            the bits we require from the next char
        limit: decideds when to change bitSize, and aids on getting the next
            query value the from the ompressed file, since limit will always 
            be 2^bitSize - 1
        binary: basically a workspace for getting bits from the char in the 
            compressed file
        k: the dictinary query
    */
    int bitsNeeded = 1, shift = 7, limit = 511;
    int binary = ( *full.begin() & 255 ) << 8 | ( *( full.begin() + 1 ) & 255 );
    int k = binary >> shift & limit;
    shift -= bitsNeeded;    
    std::string working( 1, k ), outFile = working, entry;
    for( auto i = ( full.begin() + 2 ); i != full.end(); shift -= bitsNeeded, working = entry )
    {
        // Detects a change in bit length
        if( bitsNeeded < 8 && dictSize > limit )
        {
            // decriment shift since we'll need one more bit from
            // the next char
            --shift;
            limit = limit << 1 | 1;
            ++bitsNeeded;
        }

        // Gets next query. If shift is less than 0, means the next query
        // requires three chars from the full string.
        binary = binary << 8 | ( *i++ & 255 );
        if( shift < 0 && i != full.end() )
        {
            // abs(shift) == the num of bits needed from the third char
            // so add 8 to get the proper shift value
            shift += 8;
            binary = binary << 8 | ( *i++ & 255 );
        }
        k = binary >> shift & limit;

        // if k == dictSize, means that the dictionary value for
        // that word doesn't exists yet, thus need to construct
        // one from the previous word
        if( dict.count(k) )
            entry = dict[k];
        else if( k == dictSize )
            entry = working + working[0];
        
        outFile += entry;

        if( dictSize < 65536 )
            dict[ dictSize++ ] = working + entry[0];
    }
   
    // Gets the file type of the original file. Accounts for different file type sizes,
    // Creates a string based on the file name and file type.
    std::string ext = filename.substr( filename.find('.'), filename.find_last_of('.') - filename.find('.') );
    filename = filename.substr( 0, filename.find('.') ) + "2M" + ext;

    std::ofstream filem( filename.c_str(), std::ios::binary );
    filem << outFile;
    filem.close();
}