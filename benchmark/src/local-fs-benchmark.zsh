
path=$1
out_path=$2
runs=$3

if [ -z "$path" ]; then
	echo "NO PATH (argument 1)"
	exit 1
fi

if [ -z "$out_path" ]; then
	echo "NO OUT PATH (argument 2)"
	exit 1
fi

if [ -z "$runs" ]; then
	echo "NO NUMER OF RUNS (argument 3)"
	exit 1
fi

iozone=/usr/local/bin/iozone

for i in {1..$runs}; do

	echo "Starting $i"

	# Without UBC
	$iozone -R -s1024 -s2048 -s4096 -s8192 -s16384 -s32768 -s65536 -s131072 -s262144 -c -e -I -i0 -i1 -i2 -f "$path/bench.tmp" > "$out_path/iozone-run-$i.log"

	# With UBC
	# $iozone -R -s1024 -s2048 -s4096 -s8192 -s16384 -s32768 -s65536 -s131072 -s262144 -c -i0 -i1 -i2 -f "$path/bench.tmp" > "$out_path/iozone-run-$i.log"

	echo "Finished $i"
done