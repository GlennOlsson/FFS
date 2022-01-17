
in_file="out.nosync/input"
out_file="out.nosync/output"
img_file="out.nosync/img0"

main_file="out.nosync/main.out"

encode () {
	$main_file encode $in_file > /dev/null
}

decode () {
	$main_file decode $img_file > /dev/null
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
		exit
	fi
}

setup () {
	rm -rf out.nosync/
	mkdir out.nosync
	make all || (echo "Could not make" && exit)
	r=0
}

before() {
	r=$((r + 1))
	echo "Running test $r"
	# Bind syserr to /dev/null
	rm $in_file 2> /dev/null || echo "Could not remove out.nosync/input"
	rm $out_file 2> /dev/null || echo "Could not remove out.nosync/output"
	rm $img_file.png 2> /dev/null ||  echo "Could not remove out.nosync/img"
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
echo "Bhasdas äöå /()\"23423😶‍🌫️🧑‍🍳🤣" > $in_file
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

before
# Test big file requiring splitting
cat "/Users/glenn/Desktop/Zoom-backgrounds.nosync/Scary.mp4" > $in_file
encode
$main_file decode out.nosync/img0 out.nosync/img1 out.nosync/img2 out.nosync/img3 out.nosync/img4 out.nosync/img5 out.nosync/img6
compareHash