


namespace FFS {
template <int buffer_size>
class Buffer {

	char buffer[buffer_size];

	// Can read while read_p < write_p
	int write_p = 0;
	int read_p = 0;

	/**
	 * @brief Returns how many bytes can be currently read
	 * 
	 * @return int 
	 */
	int readable();

	/**
	 * @brief Read one byte
	 * 
	 * @return char the byte
	 */
	char get();

	/**
	 * @brief Returns how many bytes can be currently written to 
	 */
	int writeable();

	/**
	 * @brief Write a byte to the stream
	 * 
	 * @param c the byte ti write 
	 */
	void write(char c);
};

}