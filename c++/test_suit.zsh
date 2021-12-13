
in_file="out/input"
out_file="out/output"
img_file="out/img.png"

encode () {
	./out/encoder $in_file
}

decode () {
	./out/decoder $img_file
}

compareSize () {
	d1=$(du $in_file)
	d2=$(du $out_file)
	if [[ $d1 == $d2 ]]; 
	then
		echo "OK"
	else
		echo "MISSMATCH BETWEEN SIZES"
		# exit
	fi
}

compareHash () {
	m1=$(md5 -q $in_file)
	m2=$(md5 -q $out_file)
	if [[ $m1 == $m2 ]]; 
	then
		echo "OK"
	else
		echo "MISSMATCH BETWEEN HASHES"
		# exit
	fi
}

setup () {
	rm -rf out/
	mkdir out
	make all || (echo "Could not make" && exit)
	r=0
}

before() {
	r=$((r + 1))
	echo "Running test $r"
	# Bind syserr to /dev/null
	rm $in_file 2> /dev/null || echo "Could not remove out/input"
	rm $out_file 2> /dev/null || echo "Could not remove out/output"
	rm $img_file 2> /dev/null ||  echo "Could not remove out/img"
}

setup

before
#
# echo "Running test 1"
# Test bytes % 6 == 0
echo "Hejsan" > $in_file
encode
decode
compareHash

before
# Test bytes % 6 == 1
echo "Hejsans" > $in_file
encode
decode
compareHash


before
# Test bytes % 6 == 2
echo "Hejsan på" > $in_file
encode
decode
compareHash


before
# Test bytes % 6 == 3
echo "Hejsan påg" > $in_file
encode
decode
compareHash


before
# Test bytes % 6 == 4
echo "Hejsans pås" > $in_file
encode
decode
compareHash


before
# Test bytes % 6 == 5
echo "Hejsans påse" > $in_file
encode
decode
compareHash
