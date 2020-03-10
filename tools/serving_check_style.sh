#!/usr/bin/env bash

function init() {
    source /root/.bashrc
    set -v
    cd Serving
}

function abort(){
    echo "Your change doesn't follow PaddlePaddle's code style." 1>&2
    echo "Please use pre-commit to check what is wrong." 1>&2
    exit 1
}

function check_style() {
    trap 'abort' 0
    set -e

    pip install cpplint 'pre-commit==1.10.4'

    export PATH=/usr/bin:$PATH
    pre-commit install
    clang-format --version

    if ! pre-commit run -a ; then
        git diff
        exit 1
    fi

    trap : 0
}

function main() {
    init
    check_style
}

main
