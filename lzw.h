// Copyright (c) 2011 Mark Nelson
//
// This software is licensed under the OSI MIT License, contained in
// the file license.txt included with this project.
//
#ifndef _LZW_DOT_H
#define _LZW_DOT_H

#include <string>
#include <unordered_map>

namespace lzw {
template<class INPUT, class OUTPUT>
void compress( INPUT &input, OUTPUT &output, const unsigned int max_code = 32767 )
{
    input_symbol_stream<INPUT> in( input );
    output_code_stream<OUTPUT> out( output, max_code );

    std::unordered_map<std::string, unsigned int> codes( (max_code * 11)/10 );
    for ( unsigned int i = 0 ; i < 256 ; i++ )
        codes[std::string(1,i)] = i;
    unsigned int next_code = 257;
    std::string current_string;
    char c;
    while ( in >> c ) {
        current_string = current_string + c;
        if ( codes.find(current_string) == codes.end() ) {
            if ( next_code <= max_code )
                codes[ current_string ] = next_code++;
            current_string.erase(current_string.size()-1);
            out << codes[current_string];
            current_string = c;
        }
    }
    if ( current_string.size() )
        out << codes[current_string];
}


template<class INPUT, class OUTPUT>
void decompress( INPUT &input, OUTPUT &output, const unsigned int max_code = 32767  )
{
    input_code_stream<INPUT> in( input, max_code );
    output_symbol_stream<OUTPUT> out( output );

    std::unordered_map<unsigned int,std::string> strings( (max_code * 11) / 10 );
    for ( int unsigned i = 0 ; i < 256 ; i++ )
        strings[i] = std::string(1,i);
    std::string previous_string;
    unsigned int code;
    unsigned int next_code = 257;
    while ( in >> code ) {
        if ( strings.find( code ) == strings.end() ) 
            strings[ code ] = previous_string + previous_string[0];
        out << strings[code];
        if ( previous_string.size() && next_code <= max_code )
            strings[next_code++] = previous_string + strings[code][0];
        previous_string = strings[code];
    }
}

}; //namespace lzw
#endif //#ifndef _LZW_DOT_H

