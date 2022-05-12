# Binary Structures
In this document we describe how the different structures are encoded into a binary format

## Inode table
```
Type		Attribute			Offset
u32			count_entries		+0
variable	entries				+4
```
Directly following this structure is `count_entries` of `Inode Entry`

### Inode Entry
```
Type		Attribute			Offset
u32			file_length			+0
u32			count_blocks		+4
u64			block_0				+8
u64			block_1				+16
...
```
`file_length` describes how many bytes the file represented by the inode entry is

The amount of blocks following `count_blocks` is the value of `count_blocks`

## Directory
```
Type		Attribute			Offset
u32			count_entries		+0
variable	dir_entry			+4
```
`count_entries` dictates how many `dir_entry` there are
### Directory Entry
```
Type		Attribute			Offset
u8			name_length			+0
cstring		name				+1
u32			inode_id			+1+name_length
```
`name_length` dictates how many characters are in the filename.

The `name` attribute is an EASCII string without termination representing the filename. 

`inode_id` points to an entry in the Inode table

## FFS Image
This section describes how an FFS image is encoded. All data stored in an image is stored like this, including the inode table, directories and file content. All the data, including the header, is encoded in the pixels of the image. The image is encoded as a PNG using 16-bit RGB values, and the header starts at the top-left most pixel, and they are aligned in a row-by-row order, i.e. the second pixel is the one right of the first one. 
### Header
Total size: 16 bytes
```
Type		Attribute			Offset
u8			'F'					+0
u8			'F'					+1
u8			'S'					+2
u8			version				+3
u64			timestamp			+4
u32			length				+12
```
The first 3 bytes of the header are constants with magic values. 

`version` describes the FFS version used to encode the image. Current version is 0, but this can be updated in the future which might change the structure of the header, and other attributes.

`timestamp` describes UNIX Epoch time when the data was encoded into the image

`length` describes how many bytes of data is encoded in the image. All data in the image might not be useful, such as filler bytes. Guaranteed to be `length ≤ height * width * 3`. When `length` bytes have been read from the image, the remaining data can be discarded.

> Note! Using Magick++, the color values are encoded into the `Quantum` data structure, and you can access the pixels of an image as an array of `Quantum`. However, this array/pointer structure CANNOT be used with a packed C-Structure due to the underlying structure of `Quantum`. On the surface, it seems and is used as if it is only a `uint16` however when for instance setting the 3 first values of the array after typecasting the array to a `uint16*`, the 3 first values in the `Quantum` array are NOT the same as what was set in the `uint16*`. I believe it has something to do with channels being encoded into the `Quantum` type

### Pixel content
Each pixel consists of 3 16-bit values; `Red`, `Green` and `Blue`. To encode 6 bytes into a pixel, the first two bytes are stored in the `Red` pixel with the first byte as the most significant byte. The next two bytes are stored in the `Green` pixel in the same way, and the last 2 in the `Blue` pixel. If less than 6 bytes needs to be added, random bytes should be assigned to the remaining bytes of the pixel. If we look at the pixels as a pointer to a 16-bit value, it can be visualized as following;
```
Type		Attribute			Offset
u8			byte_1				+0
u8			byte_2				+1
u8			byte_3				+2
u8			byte_4				+3
u8			byte_5				+4
u8			byte_6				+5
```
`byte_1` is the most significant byte of the `Red` pixel, `byte_2` is the least significant of the `Red` pixel. `byte_3` is the most significant byte of the `Blue` pixel etc. Following, `byte_7` would be the most significant byte of the `Red` attribute in the next pixel. The next pixel is the next one on the same row, or the first pixel on the next row.
