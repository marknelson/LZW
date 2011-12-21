//
// Copyright (c) 2011 Mark Nelson
//
// This software is licensed under the OSI MIT License, contained in
// the file license.txt included with this project.
//
// lzw.cpp : Defines the entry point for the console application.
//

//
// Build with gcc 4.5.2 or later, using the following command line:
//
//    g++ -std=c++0x lzw.cpp
//

#define _ITERATOR_DEBUG_LEVEL 0
#include <iostream>
#include <fstream>
#include <cstring>

#include "lzw_streambase.h"
#include "lzw-d.h"
#include "lzw.h"


void usage()
{
    std::cerr << 
        "Usage:\n"
        "lzw [-max max_code] -c input output #compress file input to file output\n"
        "lzw [-max max_code] -c - output     #compress stdin to file otuput\n"
        "lzw [-max max_code] -c input        #compress file input to stdout\n"
        "lzw [-max max_code] -c              #compress stdin to stdout\n"
        "lzw [-max max_code] -d input output #decompress file input to file output\n"
        "lzw [-max max_code] -d - output     #decompress stdin to file otuput\n"
        "lzw [-max max_code] -d input        #decompress file input to stdout\n"
        "lzw [-max max_code] -d              #decompress stdin to stdout\n";
    exit(1);
}

int main(int argc, char* argv[])
{
    int max_code = 32767;
    if ( argc >= 2 && !strcmp( "-max", argv[1] ) ) {
        if ( sscanf( argv[2], "%d", &max_code ) != 1 )
            usage();
        argc -= 2;
        argv += 2;
    }
    if ( argc < 2 )
            usage();
        bool compress;
        if ( std::string( "-c" ) == argv[1] )
            compress = true;
        else if ( std::string( "-d" ) == argv[1] )
            compress = false;
        else
            usage();
        std::istream *in = &std::cin;
        std::ostream *out = &std::cout;
        bool delete_instream = false;
        bool delete_ostream = false;
        if ( argc == 3 ) {
            in = new std::ifstream( argv[2] );
            delete_instream = true;
        }
        if ( argc == 4 ) {
            out = new std::ofstream( argv[3] );
            delete_ostream = true;
            if ( std::string("-") != argv[2] ) {
                in = new std::ifstream( argv[2] );
                delete_instream = true;
            }
        }
        if ( compress )
            lzw::compress( *in, *out, max_code );
        else
            lzw::decompress( *in, *out, max_code );
        if ( delete_instream )
            delete in;
        if ( delete_ostream )
            delete out;
    return 0;
}
