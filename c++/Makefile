compile_flags := --std=c++11 -O2 `Magick++-config --cppflags --cxxflags --ldflags --libs`
magick_flags := 

encoder: 
	g++ $(compile_flags) -o out.nosync/encoder file_system/encoder.cpp 

decoder: 
	g++ $(compile_flags) -o out.nosync/decoder file_system/decoder.cpp

inode_table:
	g++ $(compile_flags) -o out.nosync/inode_table file_system/inode_table.cpp 

all: encoder decoder inode_table