#
# Copyright (c) 2011 Mark Nelson
#
# This software is licensed under the OSI MIT License, contained in
# the file license.txt included with this project.
#
lzw: lzw.h lzw-a.h lzw-b.h lzw-c.h lzw-d.h lzw_streambase.h lzw.cpp
	g++ -std=c++0x lzw.cpp -o lzw
