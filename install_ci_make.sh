#!/usr/bin/env bash

install_main() {
    cp ci/Makefile.ci Makefile
}

install_lmdb() {
    cp ci/lmdb/Makefile.ci lib/lmdb/Makefile
}

printf "Installing Makefile for CI..."
install_main && install_lmdb

echo " [Done]"