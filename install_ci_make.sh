#!/usr/bin/env bash

install_main() {
    cp ci/Makefile.ci Makefile
}

install_lmdb() {
    cp ci/lmdb/Makefile.ci lib/lmdb/Makefile
}

echo "\n\tInstalling Makefile for CI...\n"
install_main && install_lmdb

echo "[Done]"