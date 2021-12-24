compile_flags :=  -Wall -Wunused-command-line-argument --std=c++11 -O2 

compile_flags += `Magick++-config --cppflags --cxxflags --ldflags --libs`

encoder := file_system/encoder.cpp 
decoder := file_system/decoder.cpp 
inode_table := file_system/inode_table.cpp 

encoder: 
	g++ $(compile_flags) -c -o out.nosync/encoder.o $(encoder)

decoder: 
	g++ $(compile_flags) -c -o out.nosync/decoder.o $(decoder)

inode_table: encoder decoder
	g++ $(compile_flags) -o out.nosync/inode_table $(inode_table) out.nosync/encoder.o out.nosync/decoder.o

all: encoder decoder