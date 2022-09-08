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

iozone -Ra -g 16M -s 1024k -b $binary_out -f $bench_file > $log || notify_err Benchmark failed
notify Benchmark is done