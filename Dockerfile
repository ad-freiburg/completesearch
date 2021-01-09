# A docker image for building a CompleteSearch index.
#
# See the end of the file for how to build and run.
#
# =================================================================================================

FROM alpine:3.9.4

# =================================================================================================
# Install some dependencies from the standard package manager.

RUN apk add --no-cache \
    # Bash (needed for console output etc.).
    bash \
    # Meta-package with gcc, libc-dev, etc. (needed for compiling the code).
    build-base \
    # Git (needed to download the source code of STXXL below).
    git \
    # Expat library (needed to parse XML files, e.g., dblp.xml).
    expat-dev \
    # Linux headers (needed for installing Boost below).
    linux-headers \
    # Perl (needed for e.g., producing console output).
    perl \
    # Google Hash Map (needed for the index).
    sparsehash \
    # Compression library.
    zlib-dev \
    # Debugging
    gdb \
    # Curl
    curl

# =================================================================================================
# Install some (specific) dependencies from scratch.

RUN mkdir -p /opt
WORKDIR /opt

# CompleteSearch requires STXXL <= 1.3.1.
RUN git clone --branch 1.3.1 https://github.com/stxxl/stxxl.git
RUN cd stxxl && \
    make config_gnu && \
    # Edit /utils/mlock.cpp so the code compiles with the latest version of gcc.
    sed -i '20 i #include <ctime>' utils/mlock.cpp \
    && sed -i '20 i #include <cerrno>' utils/mlock.cpp \
    && sed -i 's/sleep(86400);/nanosleep((const struct timespec[]){{0, 864000L}}, NULL);/g' utils/mlock.cpp \
    && make library_g++

# Install the Boost components 'system' and 'regex'.
RUN wget -q http://freefr.dl.sourceforge.net/project/boost/boost/1.66.0/boost_1_66_0.tar.gz
RUN tar -zxf boost_1_66_0.tar.gz
RUN cd boost_1_66_0 \
    && ./bootstrap.sh --with-libraries=system,regex \
    && ./bjam -d 0 release \
    && ./bjam -d 0 --includedir=/usr/include --libdir=/usr/lib install

# =================================================================================================

# Copy the required files into the container.
COPY ./src /src

# Build the backend.
WORKDIR /src
RUN make clean-all build-all

# The Makefile of CompleteSearch expects bash under '/usr/local/bin/bash'.
RUN ln -s /bin/bash /usr/local/bin/bash

# The name of the database to parse.
ENV DB=movies
WORKDIR /applications
EXPOSE 8080

# =================================================================================================

# Link /data (where to find the database files to parse and where to store the index files),
# and /src (where to find the compiled source code files of CompleteSearch)
# in the /databases/${DB} directory, compile the ${DB}-specific parser and build the index.
ENTRYPOINT ["/bin/bash"]

# Build and run the docker container such that it builds an index and starts the
# server, all in one command. To only start the server for an already built
# index, omit the pall (which stands for: precompute all). The value of DB
# controls, which application (subfolders in applications and data).
# 
# export DB=movies && PORT=1622 && docker build -t completesearch . && docker run -it --rm -e DB=${DB} -p ${PORT}:8080 -v $(pwd)/applications:/applications -v $(pwd)/data/:/data -v $(pwd)/ui:/ui --name completesearch.${DB} completesearch -c "make DATA_DIR=/data/${DB} DB=${DB} pall start"
