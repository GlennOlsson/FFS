# Fejk File System
> Master Thesis project by Glenn Olsson

Creating a new type of file system...

## Building and running
Build using `make all`

Run `export $(grep -v '^#' .env | xargs)` to export the environment variables. Run the program using `./out.nosync/main.out <ARG> [<ARG> ...]>`, eg. `./out.nosync/main.out fuse ffs -f -s` to mount the FUSE volume to the `./ffs` directory, and run FUSE in `follow` mode using one thread.