FROM gcc:14.2.0

RUN apt-get update && apt-get install -y \
    cmake \
    make \
    strace \
    htop \
    gdb \
    libzmq3-dev \
    # Удаляем кэшированные данные, облегчая докер
    # remove recirsive (все содержимое) force (принудительно)
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY . .


