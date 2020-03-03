# RSA Data Compression
### LZW( lzw435.cpp )
Compresses or decompresses a file using the lzw data  
compression algorithm.  
   
To Compile: g++ lzw435.cpp   
   
To Compress: ./lzw435.o c [ filename ]   
Generates [ filename ].lzw   
   
To Decompress: ./lzw435.o e [ filename ]   
Generates [ filename ]2.[ ext ]   
### Modified LZW( lzw435M.cpp )
Compresses or decompresses a file using a modified version   
of the lzw compression algorithm where bitsize of a char starts   
at 9 and increases to 16 till the dictionary is full.   
   
To Compile: g++ lzw435M.cpp    
    
To Compress: ./lzw435M.o c [ filename ]   
Generates [ filename ].lzwM   
   
To Decompress: ./lzw435M.o e [ filename ]   
Generates [ filename ]2M.[ ext ]