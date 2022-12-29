#include <fstream>
#include <iostream>

using namespace std;

void write() {
	auto stream = ofstream("ffs/tmp.txt");

	auto lim = 65530;
	char c[lim];

	for(int i = 0; i < lim; i++)
		stream.put(i & 0xFF);
	
	cout << "Wrote " << lim << " bytes" << endl;
}

void read() {
	auto stream = ifstream("ffs/tmp.txt");

	auto curr_pos = stream.tellg();

	cout << "Curr: " << curr_pos << endl;

	stream.seekg(0, stream.end);
	auto size = stream.tellg();
	cout << "Filesize: " << size << endl;

	// while(stream)
	// 	cout << (int) stream.get() << endl;

}

int main(int argc, char const *argv[]){


	write();
	read();

	return 0;
}
