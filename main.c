/*
             PUBLIC DOMAIN - 11 April MMXXIII by Jean-Marc Lienher

                The authors disclaim copyright to this software.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rijndael-alg-fst.h"

#define gadd(a, b) ((a) ^(b))
#define ROTLEFT(x,shift) ((u8) ((x) << shift | ((x) >> (8 - (shift)))))

u32 gmul(u32 a, u32 b);

/*
 * https://crypto.stackexchange.com/questions/62193/whats-happens-if-we-use-the-weakest-s-box-for-aes
 * https://www.cs.ru.nl/bachelors-theses/2011/Joost_Kremers___0714402___Practical_hacking_AES_using_the_S-box_weakness.pdf
 * https://stackoverflow.com/questions/69066821/rijndael-s-box-in-c
 */
void init_aes_sbox(u8 sbox[256]) {
    	u32 p = 1;
    	u32 q = 1;
    	u32 xf;

    	do {
        	/* multiply p by 3 */
		p = gmul(p, 3);

        	/* divide q by 3 (equals multiplication by 0xf6) */
		q = gmul(q, 0x0f6);

        	xf = q ^ ROTLEFT(q, 1) ^ ROTLEFT(q, 2) ^ 
			ROTLEFT(q, 3) ^ ROTLEFT(q, 4);

        	sbox[p] = xf ^ 0x63;

		if (1 != gmul(p, q)) {
			fprintf(stderr, "PANIC p * q != 1 in Galois field\n");
			exit(-1);
		}
    	} while (p != 1);

    	sbox[0] = 0x63; /* 0 has no inverse */
}

/*
 * https://en.wikipedia.org/wiki/Finite_field_arithmetic
 */
u32 gmul(u32 a, u32 b)
{
	u32 p = 0;
	while (a != 0 && b != 0) {
		if (b & 1) {
			p = gadd(p, a);
		}
		if (a & 0x80) {
			a = ((a << 1) ^ 0x1B) & 0xFF;
		} else {
			a = a << 1;
		}
		b = b >> 1;
	}
	return p & 0xFF;
}

void gen_tbl(char *name, u8 s[256], u8 m[4])
{
	int i;
	u32 n;
	printf("static const u32 %s[256] = {\n    ", name);
	for (i = 0; i < 256; i++) {
		n = s[i];
		printf("0x%02x%02x%02x%02xU", 
				gmul(n, m[0]),
				gmul(n, m[1]),
				gmul(n, m[2]),
				gmul(n, m[3]));
		if (i != 255) {
			printf(", ");
			if ((i & 0x3) == 3) {
				printf("\n    ");
			}
		}
	}
	printf("\n};\n");
}

int main(int argc, char *argv[])
{
	u8 S[256];
	u8 Si[256];
	int i;
	u8 m[4];

	if (argc > 1 && !strcmp(argv[1], "-tbl")) {
		init_aes_sbox(S);
		for (i = 0; i <= 255; i++) {
			Si[S[i]] = i;
		}
		m[0] = 0x2; m[1] = 0x1; m[2] = 0x1; m[3] = 0x3;
		gen_tbl("Te0", S, m);
		m[0] = 0x3; m[1] = 0x2; m[2] = 0x1; m[3] = 0x1;
		gen_tbl("Te1", S, m);
		m[0] = 0x1; m[1] = 0x3; m[2] = 0x2; m[3] = 0x1;
		gen_tbl("Te2", S, m);
		m[0] = 0x1; m[1] = 0x1; m[2] = 0x3; m[3] = 0x2;
		gen_tbl("Te3", S, m);
		m[0] = 0x1; m[1] = 0x1; m[2] = 0x1; m[3] = 0x1;
		gen_tbl("Te4", S, m);
	
		m[0] = 0xe; m[1] = 0x9; m[2] = 0xd; m[3] = 0xb;
		gen_tbl("Td0", Si, m);
		m[0] = 0xb; m[1] = 0xe; m[2] = 0x9; m[3] = 0xd;
		gen_tbl("Td1", Si, m);
		m[0] = 0xd; m[1] = 0xb; m[2] = 0xe; m[3] = 0x9;
		gen_tbl("Td2", Si, m);
		m[0] = 0x9; m[1] = 0xd; m[2] = 0xb; m[3] = 0xe;
		gen_tbl("Td3", Si, m);
		m[0] = 0x1; m[1] = 0x1; m[2] = 0x1; m[3] = 0x1;
		gen_tbl("Td4", Si, m);
		
	}
	return 0;
}

