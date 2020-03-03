/*  Algorithms: Project 3 Part 1 - Regular LZW Compression
    Brighid Harris( bah128 / 3154693 )
    11.19.2019

    This program will take two arguments, the action the user
    wants to take and the name of the file in which the the action
    will be done to.

    ./lzw435.o c [ filename ]: 
        Will compress the file with regular LZW compression.
    ./lzw435.o e [ filename ]:
        Will decompress/expand a file that has been compressed
        using reuglar LZW compression.
*/

#include <string>
#include <map>
#include <iostream>
#include <fstream>

void compressReg( std::string filename );
void decompressReg( std::string filename );

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
        compressReg( file );
    else if( test == "e" )
        decompressReg( file );
    
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
        if( code & 1 )
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

// Compresses the file using regular LZW
void compressReg( std::string filename )
{
    std::cout << "Compressing using regular lzw...\n";
    // Read File
    std::string full;
    readFile( filename, full );

    // initalize the dictionary
    int dictSize = 256;
    std::map< std::string, int > dict;
    for( int i = 0; i < dictSize; ++i )
        dict[ std::string( 1, i ) ] = i;

    std::string working = "", toVec = "", toFile = "";
    for( auto i = full.begin(); i != full.end(); ++i )
    {
        working += (char) *i;
        if( dict.count( working ) )
            toVec += *i;
        else
        {
            toFile += int2Binary( dict[toVec], 12 );
            if( dictSize < 4096 )
                dict[ working ] = dictSize++;
            working = toVec = *i;
        }
    }
    if( !toVec.empty() )
        toFile += int2Binary( dict[toVec], 12 );
    
    // Pad zeros
    if( toFile.size() % 8 != 0 )
        toFile += ( ( std::string ) "00000000").substr( 0, 8 - toFile.size() % 8 );

    // Write to the output file
    std::ofstream outFile( ( filename += ".lzw" ).c_str(), std::ios::binary );
    for( int i = 0; i < toFile.size(); i += 8 )
        outFile << ( char ) ( binary2Int( toFile.substr( i, 8 )) & 255 );
    outFile.close();
}

// Decompresses a file that was compressed using regular LZW
void decompressReg( std::string filename )
{    
    if( filename.substr( filename.find_last_of('.') ) != ".lzw" )
    {
        std::cout << filename << " was not compressed using regular lzw compression\n";
        return;
    }
    std::cout << "Decompressing regular lzw file...\n";
    // Read file
    std::string full;
    readFile( filename, full );

    // Initalize the dictionary    
    int dictSize = 256;
    std::map< int, std::string > dict;
    for( int i = 0; i < dictSize; ++i )
        dict[ i ] = std::string( 1, i );

    /* Sets initial values of helper variables
        binary: workspace for getting bits
        k: dictionary query
        flag: tells whether to read in two chars from the full 
            string or just one. Also determines whether or not
            to shift binary to get k
    */
    int binary = ( *full.begin() & 255 ) << 8 | ( *( full.begin() + 1 ) & 255 );
    int k = binary >> 4 & 4095;
    bool flag = 0;
    std::string working( 1, k ), toFile = working, entry;
    for( auto i = ( full.begin() + 2 ); i != full.end(); flag = !flag, working = entry )
    {
        // if flag, gets two chars instead of one. Shifts 4 to the left and gets those
        // 12 bits
        if( flag )
        {
            binary = ( *i++ & 255 ) << 8 | ( *i++ & 255 );
            k = binary >> 4 & 4095;
        }
        else
        {
            binary = binary << 8 | ( *i++ & 255 );
            k = binary & 4095;
        }

        if( dict.count(k) )
            entry = dict[k];
        else if( k == dictSize )
            entry = working + working[0];
        
        toFile += entry;

        if( dictSize < 4096 )
            dict[ dictSize++ ] = working + entry[0];
    }

    // Gets the file type of the original file. Accounts for different file type sizes
    std::string ext = filename.substr( filename.find('.'), filename.find_last_of('.') - filename.find('.') );
    filename = filename.substr( 0, filename.find('.') ) + '2' + ext;
    
    // Writes to the output file
    std::ofstream outFile( filename.c_str(), std::ios::binary );
    outFile << toFile;
    outFile.close();
}