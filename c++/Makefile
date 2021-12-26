compile_flags :=  -Wall --std=c++11 -O2 

compile_flags += `Magick++-config --cppflags --cxxflags --ldflags --libs`

encoder := file_system/encoder.cpp 
decoder := file_system/decoder.cpp 
inode_table := file_system/inode_table.cpp 

constants := helpers/constants.cpp 

out_dir := out.nosync

# encoder_linking: 
# 	g++ $(compile_flags) -c -o out.nosync/encoder.o $(encoder)

# decoder_linking: constants_linking
# 	g++ $(compile_flags) -c -o out.nosync/decoder.o $(decoder)

# constants_linking:
# 	g++ $(compile_flags) -c -o out.nosync/constants.o $(constants)

# inode_table: constants_linking encoder_linking decoder_linking
# 	g++ $(compile_flags) -o out.nosync/inode_table $(inode_table) out.nosync/encoder.o out.nosync/decoder.o

# all: encoder decoder

CC = g++

files := helpers/constants.cpp file_system/encoder.cpp file_system/decoder.cpp

# %.cpp:
# 	echo $@
# 	$(CC) $(compile_flags) -c $@ -o $(out_dir)/$(@F)

all: clean
	$(CC) $(compile_flags) main.cpp $(patsubst $(@F).cpp, $(out_dir)/%.o, $(files)) -o $(out_dir)/main.out 

clean:
	@rm -f $(out_dir)/*.o
