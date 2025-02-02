/*
 * Jala Alamin
 * CS 427
 * Instructor Farhana Kabir
 * Project 2
 * April 18 2020

*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include<time.h> 
#include<math.h>
#include <sys/time.h>

int decToAscii(u_int32_t m, FILE * fptr);
int keygen(unsigned long seed);
int millerRabin(unsigned long n, int s);
int witness(unsigned long a, unsigned long n);
unsigned long modularExponentiation(unsigned long a, unsigned long b, unsigned long n);
int encrypt(FILE * fp,FILE * keyfp);
int decrypt(FILE * fp, FILE * keyfp);
int encrypt_helper(u_int32_t m, unsigned long p, unsigned int g, unsigned long e2);
void doseed(void);
void checkError(FILE * fp);

int main(int argc, char *argv[]){
	FILE * fp;
	FILE * keyfp;
	int seed;
	if(argc == 2){
		if(strcmp(argv[1], "keygen" ) == 0){
			printf("Please input a seed for the random number generator: ");
			scanf("%d", &seed);
			keygen(seed);
		}
	}
	else if(argc == 4 && strncmp("-e", argv[3], 2) == 0){
		fp = fopen(argv[2], "r");
		checkError(fp);
		keyfp = fopen(argv[1], "r");
		checkError(keyfp);
		encrypt(fp, keyfp);
	}
	else if(argc == 4 && strncmp("-d", argv[3], 2) == 0){
		fp = fopen(argv[2], "r");
		checkError(fp);
		keyfp = fopen(argv[1], "r");
		checkError(keyfp);
		decrypt(fp, keyfp);
	}
	else{
		printf("Usage: \n");
		printf("./public-key-cryptosystem keygen\n");
		printf("./public-key-cryptosystem  key.txt plaintext.txt -e (for encryption)\n");
		printf("./public-key-cryptosystem  key.txt ciphertext.txt -d (for decryption)\n");
	}

	return 0;
}
void checkError(FILE * fp){
	if(fp == NULL){
		perror("Error: ");
		exit(1);
	}
}

void checkError2(FILE * fp){
	if(fp != 0){
		perror("Error: ");
		exit(1);
	}
}

int decrypt(FILE * fp, FILE * keyfp){
	FILE * fptr;
	unsigned long p, d;
	u_int32_t m;
	int g;
	unsigned long C[2];
	fscanf(keyfp, "%lx", &p) ; 
	fscanf(keyfp, "%x", &g) ; 
	fscanf(keyfp, "%lx", &d) ; 
	fptr = fopen("dtext.txt", "a");
	checkError(fptr);
	printf("Decoded Message:\n");
	while(fscanf(fp, "%ld %ld",&C[0], &C[1] ) != EOF){
		//C1 ^p-1- d·C2 mod p = m
		//(A * B) mod C = (A mod C * B mod C) mod C
		m = (modularExponentiation(C[0], (p-1-d), p) * (C[1] %p) ) % p;
		decToAscii(m, fptr);
	}
	fclose(fptr) ;
	return 0;
}
int decToAscii(u_int32_t m, FILE * fptr){
	char text;
	u_int32_t block[4];
	block[0] = (m & 0xFF000000UL) >> 24;
	block[1] = (m & 0x00FF0000UL) >> 16;
	block[2] = (m & 0x0000FF00UL) >> 8;
	block[3] = m & 0x000000FFUL;
	for(int i=0; i< 4; i++){
		text = block[i];
		printf("%c", text);
		fwrite(&text, 1,1,fptr);
	}
	return 0;
}

void doseed(void) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    unsigned seed = (unsigned)tp.tv_usec;
    srand(seed);
}
int encrypt(FILE * fp, FILE * keyfp){
	// m = 32 (block size)
	// modulo = 33 bit
	// read p, g and e2 from key file
	char buf[4];
	u_int32_t block[4];
	u_int32_t m;
	unsigned long p, e2;
	int g;
	fscanf(keyfp, "%lx", &p) ; 
	fscanf(keyfp, "%x", &g) ; 
	fscanf(keyfp, "%lx", &e2) ; 
   	int numRead;
   //reading 4 bytes or 4 characters at a time
   	printf("(C1, C2) pairs:\n");
	while((numRead =fread(buf,1,4, fp)) >0){
		for(int i=0; i< 4; i++){
			block[i] = 0;
		}
		for(int i=0; i< numRead; i++){
			block[i] = buf[i];
		}
		block[0] = block[0] << 24;
		block[1] = block[1] << 16;
		block[2] = block[2] << 8;
		m = block[0] | block[1] | block[2] | block[3];
		encrypt_helper(m, p, g, e2);
   	}
   	printf("\n");
   	return 0;

}
int encrypt_helper(u_int32_t m, unsigned long p, unsigned int g, unsigned long e2){
	FILE * fptr;
	static unsigned long C[2];
	// C1 = gk mod p
	// C2 =e2k·m mod p
	doseed();
	unsigned long k =  rand() % (p-1) ;
	C[0] = modularExponentiation(g, k, p);

	//(A * B) mod C = (A mod C * B mod C) mod C
	// e2^k * m mod p = (e2^k mod p )* (m mod p) mod p
	C[1] = ((modularExponentiation(e2,k,p) * modularExponentiation(m,1,p)))% p ;

	fptr = fopen("ctext.txt", "a");
	printf("(%lx, %lx) ", C[0], C[1]);
	fprintf(fptr, "%ld", C[0]);
	fprintf(fptr, "%c", ' ');
	fprintf(fptr, "%ld", C[1]);
	fprintf(fptr, "%c", ' ');
	fclose(fptr) ;
	return 0;
}


int keygen(unsigned long seed){
	FILE *fptr;
	unsigned long p=1,q,e2,d;
	int g = 2;
	unsigned long max = 0xFFFFFFFFUL; //gives largest number in 32 bit range
	// k= 33
	// e2 = gd mod p
	// generate safe prime P
	// select a k-1 bit prime q so that q mod 12 == 5
	// compute p = 2q + 1 and test whether p is prime
	
	while(1){
		doseed();
		q = 1 + rand() % max ;
		//truncate q to 32 bits;
		q = q &  0x00000000FFFFFFFFUL;
		//set lowest bit to 1 to ensure it is not even
		q = q | 1;

		if(q >= 0x3fffffffUL && millerRabin(q,1) == 1 && (q % 12) == 5){
			p = (2*q) + 1;
			//truncate
			p = p & 0x00000001FFFFFFFFUL;
			//p = p | 0x80000000UL;
			//check that p's last bit is 1
			unsigned long bit = p & 0x000000001UL;
			if(bit == 1 ){
				if(millerRabin(p,1) == 1) break;
			}
		}
	}
	srand(seed);
	d = 1 + rand() % p-2;
	//e1^d % p
	e2 = modularExponentiation(g, d, p);
	
	fptr = fopen("pubkey.txt", "w+");
	fprintf(fptr, "%lX", p);
	fprintf(fptr, "%c", ' ');
	fprintf(fptr, "%X", g);
	fprintf(fptr, "%c", ' ');
	fprintf(fptr, "%lX", e2);
	fclose(fptr) ;
	fptr = fopen("prikey.txt", "w+");
	fprintf(fptr, "%lX", p);
	fprintf(fptr, "%c", ' ');
	fprintf(fptr, "%X", g);
	fprintf(fptr, "%c", ' ');
	fprintf(fptr, "%lX", d);
	fclose(fptr) ;
	printf("p  = %08lx\ng  = %d\ne2 = %08lx\nd  = %08lx\n", p,g,e2,d);
	return 0;
}

/* return 1 if prime, 0 for composite  */
int millerRabin(unsigned long n, int s){
	int a;

	for(int j=1; j<=s;j++){
		doseed();
		a= (rand() % ((n-1) - 1 + 1)) + 1; 
		if(witness(a, n) == 1){
			return 0;
		}
	}
	return 1;
}
int witness(unsigned long a, unsigned long n){
	int t = 1;
	unsigned long n2 = n-1, u;
	unsigned long prevx, x, factor;
	while(1){
		factor = pow(2,t);
		if(n2 % factor != 0){
			break;
		}
		u = n2/pow(2,t);
		t++;
	}
	prevx = modularExponentiation(a, u, n);
	for(int i=1; i<=t; i++){
		x = (prevx *prevx) % n;
		if(x ==1 && prevx !=1 && prevx !=n-1){
			return 1;
		}
		prevx = x;
	}
	if(x != 1){
		return 1;
	}
	return 0;
}
unsigned long modularExponentiation(unsigned long a, unsigned long b, unsigned long n){
	int c = 0;
	int bit = 0;
	unsigned long tmp, d=1;

	for(int i=0; i< 64; i++){
		tmp = b & 0x8000000000000000UL;
		bit = tmp >>63;
		c = 2*c;
		d = (d*d) % n;
		if(bit == 1){
			c++;
			d = (d*a) % n;
		}
		b = b << 1;
		//printf("bit is %d, c = %d, d= %d\n",bit, c, d);
	}
	return d;
}