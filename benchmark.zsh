if [ -z "$1" ]; then
    echo "1st arugment needs to be benchmark identifier, eg. \"flickr\""
	exit 1
fi

bench_file=$2
# If there is a second paramter, use this for bench file. Else, use ffs/bench.tmp as default
if [ -z "$2" ]; then
    echo "2nd arugment needs to be the benchmark file"
	exit 1
fi

base="/Volumes/TimeCapsule/FFS"

for i in {1..20}; do
    echo "Starting run $i"
    dir="$base/run$i"
    mkdir $dir

    log="$dir/$1.log"

    # tshark -i en9 -w "$dir/$1-trace.pcapng" -f "predef:TCP HTTP or HTTPS" &

    # tshark_pid=$!

     
	#  time iozone -R -s1024 -s2048 -s4096 -s8192 -s16384 -s32768 -s65536 -s131072 -c -e -I -i0 -i1 -i2 -f $2 > $log || notify_err Benchmark failed

	#  time iozone -R -s1024 -s2048 -s4096 -s8192 -s16384 -s32768 -s65536 -s131072 -s262144 -c -e -I -i0 -i1 -i2 -f $2 > $log || notify_err Benchmark failed

    time iozone -R -s1024 -s2048 -s4096 -s8192 -s16384 -s32768 -s65536 -s131072 -s262144 -c -i0 -i1 -i2 -f $2 > $log || notify_err Benchmark failed

    date
    echo "Finished run $i"
    
    return_code=$?
    if [ $return_code -ne 0 ]; then
        echo "Bad run $i"
    fi

    # kill $tshark_pid
done

notify Benchmark is done