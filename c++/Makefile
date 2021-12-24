encoder: 
	g++ --std=c++11 -O2 -o out.nosync/encoder file_coding/encoder.cpp `Magick++-config --cppflags --cxxflags --ldflags --libs`

decoder: 
	g++ --std=c++11 -O2 -o out.nosync/decoder file_coding/decoder.cpp `Magick++-config --cppflags --cxxflags --ldflags --libs`

all: encoder decoder