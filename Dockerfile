FROM ubuntu:latest

RUN apt-get update
RUN apt-get upgrade

RUN apt-get -y install build-essential
RUN apt-get -y install autoconf automake gdb git libffi-dev zlib1g-dev libssl-dev
# Magick++
# RUN apt-get -y install libgraphicsmagick1-dev  imagemagickapt-get libmagick++-dev  
# RUN apg-get -y install rpm
RUN curl -o "ImageMagick.tar.gz" "https://imagemagick.org/archive/ImageMagick.tar.gz"
RUN tar -xf ImageMagick.tar.gz 
RUN cd ImageMagick-7.1.0-57
RUN ./configure
RUN make
RUN make install
RUN cd ..

# Fuse
RUN apt-get -y install libfuse2

RUN apt-get -y install curl
#libcurl
RUN apt-get -y install libcurl4-openssl-dev
# Download and build curlpp
RUN curl -o curlpp.tar "https://github.com/jpbarrette/curlpp/archive/refs/tags/v0.8.1.tar.gz"
RUN tar -xf curlpp.tar
RUN cd curlpp-0.8.1
RUN cmake .
RUN make
# Symlink the include dir
RUN ln -s /src/curlpp-0.8.1/include/ /usr/local/include/curlpp
RUN ln -s /src/curlpp-0.8.1/include/utilspp/ /usr/local/include/utilspp
# Symlink the .pc file for the pkg-config command to work
RUN export PKG_CONFIG_PATH=/usr/local/lib
RUN ln -s /src/curlpp-0.8.1/extras/curlpp.pc /usr/local/lib/curlpp.pc

RUN cd ..

# Download and build liboauth
RUN apt-get -y install unzip
RUN curl -o liboauthcpp.zip "https://codeload.github.com/sirikata/liboauthcpp/zip/refs/heads/master"
RUN unzip liboauthcpp.zip
RUN cd liboauthcpp-master
RUN cd build
RUN cmake .
RUN make
# Add include dir and executable
RUN ln -s /src/liboauthcpp-master/include/liboauthcpp /usr/local/include/liboauthcpp
RUN ln -s /src/liboauthcpp-master/build/liboauthcpp.a /usr/local/lib/liboauthcpp.a 

RUN cd ..

# Flickcurl
RUN curl -o flickcurl.tar.gz "https://download.dajobe.org/flickcurl/flickcurl-1.26.tar.gz"
RUN tar -xf flickcurl.tar.gz
RUN cd flickcurl-1.26
RUN apt-get -y install gtk-doc-tools
RUN ./configure
RUN make
RUN make install

# crypto++
RUN apt-get -y install libcrypto++8 libcrypto++-dev


CMD [ "/bin/bash" ]