// Copyright (c) 2011 Mark Nelson
//
// This software is licensed under the OSI MIT License, contained in
// the file license.txt included with this project.
//
#ifndef LZW_B_DOT_H
#define LZW_B_DOT_H

#include "lzw_streambase.h"
#include <iostream>

//
// lzw-b specializes the four I/O classes for std::istream and std::ostream.
// The symbol I/O classes are identical to those in lzw-a.h, but the code
// I/O classes have to be completely different.
//

namespace lzw {

//
// It's tempting to try to read characters using the ifstream
// extraction operator, as in m_impl >> c, but that operator
// skips over whitespace, so we don't get an exact copy of 
// the input stream. Using get() works around this problem.
//
template<>
class input_symbol_stream<std::istream> {
public :
    input_symbol_stream( std::istream &input ) 
        : m_input( input ) {}
    bool operator>>( char &c )
    {
        if ( !m_input.get( c ) )
            return false;
        else
            return true;
    }
private :
    std::istream &m_input;
};
//
// Using the insertion operator to output strings seems to work properly,
// even when the strings contain binary data, so this implementation is
// as simple as we could hope for.
//
template<>
class output_symbol_stream<std::ostream> {
public :
    output_symbol_stream( std::ostream &output ) 
        : m_output( output ) {}
    void operator<<( const std::string &s )
    {
        m_output << s;
    }
private :
    std::ostream &m_output;
};

//
// Writing the codes to std::ostream as binary values requires breaking
// the integer code into two bytes and writing the bytes one at a time. There are
// more efficient ways to write the complete short integer in one
// function call, but they raise code portability problems, as we
// don't always know what order bytes will be written in.
//
template<>
class output_code_stream<std::ostream> {
public :
    output_code_stream( std::ostream &output, const int ) 
        : m_output( output ) {}
    void operator<<( unsigned int i )
    {
        m_output.put( i & 0xff );
        m_output.put( (i>>8) & 0xff);
    }
    ~output_code_stream()
    {
        *this << EOF_CODE;
    }
private :
    std::ostream &m_output;
};
//
// Reading the codes requires reading the
// two bytes that make up the short integer,
// then combining them. While reading, if the
// routine detects an EOF, it returns false,
// which tells the decompressor to stop working.
// It also returns false if there is an error
// on the input stream.
//
template<>
class input_code_stream<std::istream> {
public :
    input_code_stream( std::istream &input, unsigned int ) 
        : m_input( input ) {}
    bool operator>>( unsigned int &i )
    {
        char c;
        if ( !m_input.get(c) )
            return false;
        i = c & 0xff;
        if ( !m_input.get(c) )
            return false;
        i |= (c & 0xff) << 8;
        if ( i == EOF_CODE )
            return false;
        else
            return true;
    }
private :
    std::istream &m_input;
};

}; //namespace lzw


#endif //#ifndef LZW_B_DOT_H

