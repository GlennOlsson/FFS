


namespace FFS {
class Buffer {
	int buffer_size;
	char[] buffer;

	// Can read while read_p < write_p
	int write_p;
	int read_p;

	/**
	 * @brief Read one byte
	 * 
	 * @return char the byte
	 */
	char get() {
		int new_read_p = (read_p + 1) % buffer_size;
	}
};
