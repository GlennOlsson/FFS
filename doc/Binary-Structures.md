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