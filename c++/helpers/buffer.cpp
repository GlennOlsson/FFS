#include "buffer.h"

template <int buffer_size>
int FFS::Buffer<buffer_size>::readable() {
	return this->write_p - this->read_p;
}

template <int buffer_size>
char FFS::Buffer<buffer_size>::get() {
	return buffer[read_p++];
}

template <int buffer_size>
int FFS::Buffer<buffer_size>::writeable() {
	return this->buffer_size - this->write_p;
}

template <int buffer_size>
void FFS::Buffer<buffer_size>::write(char c) {
	buffer[write_p++] = c;
}