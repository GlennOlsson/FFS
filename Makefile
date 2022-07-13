compile_flags :=  -Wall --std=c++20 -O2 

magick_flags := `Magick++-config --cppflags --cxxflags --ldflags --libs`
fuse_flags = `pkg-config --cflags --libs fuse`

out_dir := out.nosync
test_dir := tests

CC = g++

files := src/**/*.cpp
test_files := $(test_dir)/*.cpp
test_main := $(out_dir)/main_test.out

fuse_mount_point = ffs

$(out_dir):
	@mkdir $(out_dir)

all: | $(out_dir)
	@$(CC) $(compile_flags) $(magick_flags) $(fuse_flags) src/main.cpp $(patsubst $(@F).cpp, $(out_dir)/%.o, $(files)) -o $(out_dir)/main.out 

# Keep alive (-f), disable multi-threading (-s), debugging (-d)
fuse: clean_fuse all
	./$(out_dir)/main.out fuse $(fuse_mount_point) -f

main_test: | $(out_dir)
	@$(CC) $(compile_flags) -c $(test_dir)/main/main_test.cpp -o $(test_main)

all_tests: all | $(out_dir)
	@$(CC) $(compile_flags) $(magick_flags) $(fuse_flags) $(test_main) $(patsubst $(@F).cpp, $(out_dir)/%.o, $(test_files)) $(patsubst $(@F).cpp, $(out_dir)/%.o, $(files)) -o $(out_dir)/test.out 
	$(out_dir)/test.out

clean: | $(out_dir)
	@rm -f $(out_dir)/*.out

clean_fuse:
	-@umount $(fuse_mount_point)
	-@rmdir $(fuse_mount_point)
