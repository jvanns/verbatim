##
# Dockerfile responsible for installing build environment for verbatim
##

FROM ubuntu:latest

MAINTAINER Jim Vanns <james.vanns@gmail.com>

##
# Set any environment necessary
##
ENV LC_ALL C
ENV LANG en_GB.UTF-8
ENV LANGUAGE en_GB.UTF-8

ENV TERM linux
ENV DEBIAN_FRONTEND noninteractive
ENV PATH /bin:/usr/bin:/sbin:/usr/sbin

##
# Build commands. Put each logically-grouped set of
# commands in a single shell pipeline. Not only is
# this good practice and helps maintain readability
# but it also helps the intermediate build stages
# remain cached when changing another (to fix it!).
##

# Locale
RUN \
   apt-get update && \
   apt-get -y install "language-pack-${LANG%%_*}" && \
   locale-gen "$LANG" && \
   update-locale LANG="$LANG"

# Update
RUN \
   apt-get update && \
   apt-get -y upgrade && \
   apt-get -y dist-upgrade && \
   apt-get -y autoremove

# Install dependencies
RUN \
   apt-get update && \
   apt-get -y install \
      vim git make exuberant-ctags clang-3.5 libstdc++-4.8-dev \
      linux-tools-generic valgrind strace gdb libstdc++-4.8-doc \
      libboost1.55-dev libboost1.55 libtag1-dev libtag1-doc

# Add this source directory
ENV VERBATIM_ROOT /usr/src/verbatim
ADD . $VERBATIM_ROOT

WORKDIR $VERBATIM_ROOT
RUN \
   ulimit -c 100000000 && \
   make -j`getconf _NPROCESSORS_ONLN` all
ENV LD_LIBRARY_PATH ${VERBATIM_ROOT}/sub/lmdb/libraries/liblmdb
