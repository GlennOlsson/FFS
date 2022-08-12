compile_flags :=  -Wall --std=c++20 -O2 

magick_flags := `Magick++-config --cppflags --cxxflags --ldflags --libs`
fuse_flags := `pkg-config --cflags --libs fuse`
curl_flags := `pkg-config --cflags curlpp` -lcurlpp -lcurl
flickr_flags := `flickcurl-config  --cflags` `flickcurl-config --libs`
cryptpp_flags = -I /usr/local/include/cryptopp -L/usr/local/lib -lcryptopp
oathflags = -L/usr/local/lib -loauthcpp -I/usr/local/include/lliboauthcpp

all_flags := $(compile_flags) $(magick_flags) $(fuse_flags) $(curl_flags) $(oathflags) $(cryptpp_flags) $(flickr_flags)

out_dir := out.nosync
test_dir := tests

CPP_CC := g++
C_CC := gcc


cpp_files := src/**/*.cpp
c_file := src/json/json.c
test_files := $(test_dir)/*.cpp
test_main := $(out_dir)/main_test.out

fuse_mount_point = ffs

$(out_dir):
	@mkdir $(out_dir)

all: | $(out_dir)
	@$(C_CC $(c_files) -c -o out.nosync/c.out
	@$(CPP_CC) $(all_flags) src/main.cpp out.nosync/c.out $(patsubst $(@F).cpp, $(out_dir)/%.o, $(cpp_files)) -o $(out_dir)/main.out  || notify_err FFS compile failed
	@notify FFS compile is done

# Keep alive (-f), disable multi-threading (-s), debugging (-d)
fuse: clean_fuse all
	./$(out_dir)/main.out fuse $(fuse_mount_point) -f

main_test: | $(out_dir)
	@$(CPP_CC) $(compile_flags) -c $(test_dir)/main/main_test.cpp -o $(test_main)
	@notify FFS Main Test has been compiled

all_tests: all | $(out_dir)
	@time $(CPP_CC) $(all_flags) $(test_main) $(patsubst $(@F).cpp, $(out_dir)/%.o, $(test_files)) $(patsubst $(@F).cpp, $(out_dir)/%.o, $(cpp_files)) -o $(out_dir)/test.out 
	time $(out_dir)/test.out || notify_err FFS test failed
	@notify FFS testing is done

clean: | $(out_dir)
	@rm -f $(out_dir)/*.out

clean_fuse:
	-@umount $(fuse_mount_point)
	-@rmdir $(fuse_mount_point)
