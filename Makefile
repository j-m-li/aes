
all:
	cc -o aes.exe rijndael-alg-fst.c main.c

clean:
	rm -f aes.exe

