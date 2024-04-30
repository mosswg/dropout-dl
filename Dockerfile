FROM alpine

RUN echo "https://dl-cdn.alpinelinux.org/alpine/edge/main" >> /etc/apk/repositories && \
	echo "https://dl-cdn.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories && \
	echo "https://dl-cdn.alpinelinux.org/alpine/edge/community" >> /etc/apk/repositories

RUN apk update && \
	apk --no-cache add make gcc g++ musl-dev curl curl-dev git cmake make ffmpeg

# Copy application now
WORKDIR /app
COPY ./ /app

RUN git submodule update --init --recursive
RUN cmake -S /app -B build
WORKDIR /app/build
RUN make && \
	chmod +x dropout-dl && \
	cp dropout-dl ../

WORKDIR /app

ENTRYPOINT [ "/app/dropout-dl" ]
