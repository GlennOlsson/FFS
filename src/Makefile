compile_flags :=  -Wall --std=c++20 -O2 

magick_flags := `Magick++-config --cppflags --cxxflags --ldflags --libs`

encoder := file_system/encoder.cpp 
decoder := file_system/decoder.cpp 
inode_table := file_system/inode_table.cpp 

constants := helpers/constants.cpp 

out_dir := out.nosync
test_dir := tests

CC = g++

files :=  file_system/*.cpp helpers/*.cpp
test_files := $(test_dir)/*.cpp
test_main := $(out_dir)/main_test.out

$(out_dir):
	@mkdir $(out_dir)

all: | $(out_dir)
	@$(CC) $(compile_flags) $(magick_flags) main.cpp $(patsubst $(@F).cpp, $(out_dir)/%.o, $(files)) -o $(out_dir)/main.out 

main_test: | $(out_dir)
	@$(CC) $(compile_flags) -c $(test_dir)/main/main_test.cpp -o $(test_main)

all_tests: all | $(out_dir)
	@$(CC) $(compile_flags) $(magick_flags) $(test_main) $(patsubst $(@F).cpp, $(out_dir)/%.o, $(test_files)) $(patsubst $(@F).cpp, $(out_dir)/%.o, $(files)) -o $(out_dir)/test.out 
	$(out_dir)/test.out

clean: | $(out_dir)
	@rm -f $(out_dir)/*.out
