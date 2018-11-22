CC := ./node_modules/webassembly/bin/wa

CFLAGS :=

OBJS := models.wasm funcs.wasm

main.wasm: #$(OBJS)
	$(CC) compile $(CFLAGS) -o $@ funcs.c
	#./node_modules/webassembly/tools/bin/linux-x64/wasm-merge -o $@ $^

%.wasm: %.c *.h
	$(CC) compile $(CFLAGS) -o $@ $<
