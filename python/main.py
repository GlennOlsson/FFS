from PIL import Image  
import random
import sys
import math

#img = Image.new(mode = "RGB",size=(width, height))

colors = [
	255, 255, 255,
	255, 0, 0,
	0, 255, 0,
	0, 0, 255
]
# while len(colors) < 3 * (width * height):
# 	r = random.randint(0, 255)
# 	g = random.randint(0, 255)
# 	b = random.randint(0, 255)

	# colors.extend([r, g, b])

input_file_path = "out/input"
if len(sys.argv) > 1:
	input_file_path = sys.argv[1]

input_file = open(input_file_path, mode="br")

file_bytes = input_file.read()

# +1 as first pixel will be info about how many pixels are nonsense
min_pixels = (len(file_bytes) / 3) + 1

# The number of pixels needed to encode file, upper limit. Could need 1 or 2 bytes less
pixels_for_file = math.ceil(len(file_bytes) / 3)

sqrt_pixels = math.sqrt(min_pixels)

closest_root = int(sqrt_pixels) if int(sqrt_pixels) == sqrt_pixels else int(sqrt_pixels) + 1


# width * height - min_pixels == pixels we need to add
# len(file_bytes) % 3 to get how many bytes we need to add in last pixel
# bytes_to_add = 3 * (closest_root ** 2) - (int(min_pixels) + len(file_bytes) % 3)

# -1 because don't need to add bytes for lenght encoding
bytes_to_add = ((closest_root ** 2) * 3 - len(file_bytes)) - 3

# rgb=(b1, b2, b3) where b1 is most significant byte of number etc. (i.e. left most byte)
b1 = (len(file_bytes) >> (2*8)) & 0xff
b2 = (len(file_bytes) >> (1*8)) & 0xff
b3 = (len(file_bytes) >> (0*8)) & 0xff

total_bytes = bytearray()
total_bytes.extend([b1, b2, b3])
total_bytes.extend(file_bytes)
total_bytes.extend([random.randint(0, 255) for _ in range(bytes_to_add)])

assert len(total_bytes) / 3 == closest_root ** 2

img = Image.frombytes("RGB", (closest_root, closest_root), bytes(total_bytes), "raw", "RGB")

px = img.load()
# px[0,0] = (255, 0, 0)

img.save('out/image.png')#, format='JPEG', subsampling=0, quality=100)