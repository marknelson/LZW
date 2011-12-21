//
// Copyright (c) 2011 Mark Nelson
//
// This software is licensed under the OSI MIT License, contained in
// the file license.txt included with this project.
//
#ifndef LZW_A_DOT_H
#define LZW_A_DOT_H

#include "lzw_streambase.h"
#include <iostream>

//
// lzw-a implements all four classes for I/O using std::istream and
// std::ostream. The I/O clases for codes write the integer values
// using their textual representations separated by newlines. This
// is not much good for compression ratios, but it is a very good
// way to be able to debug the output stream, as it can be loaded
// into a text editor.
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
// LZW-A prints the text values of integers to the output
// stream, and reads them back in that format. This is not
// efficient at all, but it is much easier to debug. If
// you are having a problem with the algorithm, this provides
// a great way to examine your stream. The implementation
// of this is very simple - just using the std::ostream
// insertion operator, and following each code by a newline
// so it can properly parse on input, as well as be easilyr loaded
// into a text editor.
//
// One important thing to notice in this class: the presence of 
// a destructor that prints the EOF_CODE. Since this object goes
// out of scope as the compressor exists, this insures that every
// code stream will end with this special code. Putting the onus on 
// the I/O routines to deal with EOF issues simplifies the
// algorithm itself.
//
template<>
class output_code_stream<std::ostream> {
public :
    output_code_stream( std::ostream &output, const int ) 
        : m_output( output ) {}
    void operator<<( unsigned int i )
    {
        m_output << i << '\n';
    }
    ~output_code_stream()
    {
        *this << EOF_CODE;
    }
private :
    std::ostream &m_output;
};

//
// The corresponding version of the input operator
// just reads in the white-space separated codes.
// If there is an error or an EOF_CODE encountered
// in the stream, the extraction operator returns 
// false, which allows the decompressor to know
// when it is time to stop processing.
//
template<>
class input_code_stream<std::istream> {
public :
    input_code_stream( std::istream &input, unsigned int ) 
        : m_input( input ) {}
    bool operator>>( unsigned int &i )
    {
        m_input >> i;
        if ( !m_input || i == EOF_CODE )
            return false;
        else
            return true;
    }
private :
    std::istream &m_input;
};

}; //namespace lzw


#endif //#ifndef LZW_A_DOT_H

