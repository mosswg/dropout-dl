FROM alpine

RUN echo "https://dl-cdn.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories && \
	echo "https://dl-cdn.alpinelinux.org/alpine/edge/community" >> /etc/apk/repositories

RUN apk update && \
	apk add curl curl-dev alpine-sdk cmake make

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
