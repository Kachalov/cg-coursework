CC := ./node_modules/webassembly/bin/wa

CFLAGS :=

FILES := *.c
#OBJS := models.wasm funcs.wasm

main.wasm: $(FILES)
	cat $^ > main.c
	-$(CC) compile $(CFLAGS) -o $@ main.c
	-mv main.c main.c.all
	#./node_modules/webassembly/tools/bin/linux-x64/wasm-merge -o $@ $^

#%.wasm: %.c *.h
#	$(CC) compile $(CFLAGS) -o $@ $<
