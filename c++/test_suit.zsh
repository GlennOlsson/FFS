
in_file="out/input"
out_file="out/output"
img_file="out/img0.png"

encode () {
	./out/encoder $in_file > /dev/null
}

decode () {
	./out/decoder $img_file > /dev/null
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
echo "HejsanHejsanHejsanHejsanHejsanHejsan" > $in_file
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
echo "Hejsan pa" > $in_file
encode
decode
compareHash


before
# Test bytes % 6 == 3
echo "Hejsan pag" > $in_file
encode
decode
compareHash


before
# Test bytes % 6 == 4
echo "Hejsans pas" > $in_file
encode
decode
compareHash


before
# Test bytes % 6 == 5
echo "Hejsans pase" > $in_file
encode
decode
compareHash


before
# Test non-ascii characters
echo "Bhasdas äöå /()\"23423" > $in_file
encode
decode
compareHash


before
# Test pdf file
cat "/Users/glenn/Documents/KTH/CalPoly/Thesis/Magick++_tutorial.pdf" > $in_file
encode
decode
compareHash


before
# Test jpg file
cat "/Users/glenn/Desktop/Zoom-backgrounds.nosync/20210622-224503-krigun.jpg" > $in_file
encode
decode
compareHash