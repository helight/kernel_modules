CC=clang
LIBS=-lelf -lbpf
#-static

all: bpf_loader bpf_program.o

bpf_program.o: bpf_program.c
	$(CC) -target bpf -g -O2 -c bpf_program.c -o bpf_program.o
	
bpf_loader.o: bpf_loader.c
	$(CC) -c bpf_loader.c -o bpf_loader.o

bpf_loader: bpf_loader.o
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm *.o bpf_loader
