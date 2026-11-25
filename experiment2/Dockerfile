FROM archlinux:latest

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
        base-devel \
        git \
        wget \
        clang \
        crystal \
        shards \
        lld \
        ruby \
        tcc \
        zig \
        vim less mc \
    && pacman -Scc --noconfirm

ENV CC=clang
WORKDIR /opt

RUN git clone https://github.com/kostya/qbe.git && \
    make -C qbe

ENV PATH="/opt/qbe:${PATH}"

RUN git clone https://github.com/michaelforney/cproc.git && \
    make -C cproc

ENV PATH="/opt/cproc:${PATH}"

RUN git clone https://github.com/kostya/myc.git

ENV PATH="/opt/myc:${PATH}"

ARG gm_version="0.11.1"
RUN mkdir -p /opt/myc/plugins/cppgm && \
    wget -qO- https://github.com/vishvananda/cppgm/releases/download/v${gm_version}/cppgm-v${gm_version}-linux-x86_64.tar.gz | \
    tar -xz -C /opt/myc/plugins/cppgm --strip-components=1

ENV PATH="/opt/myc/plugins/cppgm/bin:${PATH}"

ARG myc_version="v0.13.0"
RUN cd /opt/myc && \
    git fetch && \
    git checkout ${myc_version} && \
    shards && \
    crystal build src/cli/mycc.cr --release -o ./mycc

RUN git clone https://codeberg.org/lsof/antcc && \
    cd antcc && \
    ./configure && \
    make && \
    make install

ARG madc_version=0.106.1
RUN mkdir -p /opt/madc && \
    wget -qO- https://github.com/derekbsnider/madc/releases/download/v$madc_version/madc-$madc_version-linux-x86_64.tar.gz | \
    tar -xz -C /opt/madc --strip-components=1
ENV PATH="/opt/madc/bin:$PATH"

RUN curl -Lo mucc https://github.com/Dom-Yan/Mucc/releases/latest/download/mucc-$(uname -m)-linux \
    && chmod +x mucc

ENV PATH="/opt:$PATH"
