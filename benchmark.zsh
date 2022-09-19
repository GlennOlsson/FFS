if [ -z "$1" ]; then
    echo "1st arugment needs to be benchmark identifier, eg. \"flickr\""
	exit 1
fi

bench_file=$2
# If there is a second paramter, use this for bench file. Else, use ffs/bench.tmp as default
if [ -z "$2" ]; then
    bench_file="ffs/bench.tmp"
fi

log="$1.log"
binary_out="$1.wks"

time iozone -R -s1024 -s2048 -s4096 -s8192 -s16384 -c -i0 -i1 -i2 -b $binary_out -f $bench_file > $log || notify_err Benchmark failed
notify Benchmark is done