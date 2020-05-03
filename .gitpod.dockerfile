FROM gitpod/workspace-full-vnc

RUN sudo sh -c "echo deb-src http://archive.ubuntu.com/ubuntu/ focal main restricted >> /etc/apt/sources.list" \
 && sudo sh -c "echo deb-src http://archive.ubuntu.com/ubuntu/ focal-updates main restricted >> /etc/apt/sources.list" \
 && sudo sh -c "echo deb-src http://security.ubuntu.com/ubuntu/ focal-security main restricted >> /etc/apt/sources.list" \
 && sudo sh -c "echo deb-src http://security.ubuntu.com/ubuntu/ focal-security universe >> /etc/apt/sources.list" \
 && sudo sh -c "echo deb-src http://security.ubuntu.com/ubuntu/ focal-security multiverse >> /etc/apt/sources.list" \
 && sudo sh -c "echo deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal main >> /etc/apt/sources.list.d/llvm.list" \
 && sudo apt-get update \
 && sudo apt-get install -y \
    build-essential git libkrb5-dev graphviz nasm \
    valgrind libpoco-dev libpng-dev libcap-dev python3-polib libpam0g-dev libgtk2.0-dev \
    clang-format clang-tidy clang-tools clang clangd libc++-dev libc++1 libc++abi-dev libc++abi1 libclang-dev libclang1 \
    liblldb-dev lld lldb llvm-dev llvm-runtime llvm \
 && sudo apt-get build-dep -y libreoffice \
 && sudo rm -rf /var/lib/apt/lists/*

