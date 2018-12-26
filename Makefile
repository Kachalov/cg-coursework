CC := ./node_modules/webassembly/bin/wa

CFLAGS :=

FILES := *.c

IMG_TAG := 27122018.1
DOCKER_TAG := kachalov/cg:$(IMG_TAG)

build: $(FILES)
	cat $^ > main.c
	-$(CC) compile $(CFLAGS) -o main.wasm main.c
	-mv main.c main.c.all

deps:
	npm -i webassembly
	cp ./node_modules/webassembly/dist/webassembly.js ./webassembly.js

docker:
	docker build -t cg .
	docker tag cg $(DOCKER_TAG)

push:
	docker push $(DOCKER_TAG)
