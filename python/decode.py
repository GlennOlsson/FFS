from PIL import Image  
import random
import io
import functools
import sys

input_file_path = "out/image.png"
if len(sys.argv) > 1:
	input_file_path = sys.argv[1]

img = Image.open(input_file_path, "r")

# byte_array = io.BytesIO()
# img.save(byte_array, format=img.format)
# byte_array_values = byte_array.getvalue()

b = img.tobytes()

b1 = b[0]
b2 = b[1]
b3 = b[2]

file_bytes = (b1 << 8*2) | (b2 << 8*1) | (b3 << 8*0)

# Start after byte 3 and only read file_bytes of content, + 3 as we started at byte 3
file_content = b[3:file_bytes + 3]

out_file = open("out/decoded", "wb")
out_file.write(file_content)

# img.save("out/copy.jpeg")

# input_file = open("out/image.jpeg", "br")

# b = input_file.read()
# height = int(len(b) / 3)

# print("bytes:", len(b), height)

# print(b)

# img = Image.frombytes("RGB", (1, height), b, "raw", "RGB")