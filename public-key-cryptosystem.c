/*
 * Jala Alamin
 * CS 427
 * Instructor Farhana Kabir
 * Project 2
 * April 2020

*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include<time.h> 

int keygen(unsigned long seed);
int millerRabin(unsigned long n, int s);
int witness(unsigned long a, unsigned long n);
unsigned long modularExponentiation(unsigned long a, unsigned long b, unsigned long n);


int main(int argc, char *argv[]){
	int seed;
	if(argc == 2){
		if(strcmp(argv[1], "keygen" ) == 0){
			printf("Please input a seed for the random number generator: ");
			scanf("%d", &seed);
			keygen(seed);
		}
	}

	return 0;
}

int keygen(unsigned long seed){
	clock_t t; 
    t = clock(); 
	FILE *fptr;
	unsigned long p=1,q,e2,d;
	int g = 2;
	unsigned long max = 0x7FFFFFFFUL; //gives largest number in 31 bit range
	// k= 32
	//e2 = gd mod p
	//generate safe prime P
	// select a k-1 bit prime q so that q mod 12 == 5
	// compute p = 2q + 1 and test whether p is prime
	while(1){
		srand(time(NULL));
		q = 1 + rand() % max ;
		//truncate q to 31 bits;
		q = q &  0x000000007FFFFFFFUL;
		//set lowest bit to 1 to ensure it is not even
		q = q | 1;

		if(q >= 0x3fffffffUL && millerRabin(q,1) == 1 && (q % 12) == 5){
			p = (2*q) + 1;
			//set high bit to 1 to ensure large enough
			//truncate
			p = p & 0x00000000FFFFFFFFUL;
			//p = p | 0x80000000UL;
			//check that p's 32nd bit is 1, plus truncate to 32 bits
			unsigned long bit = p & 0x00000001UL;
			if(bit == 1 ){
				//printf("odd p=%08lx\n", p);
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
	fclose(fptr);
	fptr = fopen("prikey.txt", "w+");
	fprintf(fptr, "%lX", p);
	fprintf(fptr, "%c", ' ');
	fprintf(fptr, "%X", g);
	fprintf(fptr, "%c", ' ');
	fprintf(fptr, "%lX", d);
	fclose(fptr);
	t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  
    printf("kegen() took %f seconds to execute \n", time_taken); 
	printf(" g=%d, e2=%08lx, p = %08lx,d = %08lx\n", g,e2,p,d);
	return 0;
}

/* return 1 if prime, 0 for composite  */
int millerRabin(unsigned long n, int s){
	int a;
	for(int j=1; j<=s;j++){
		srand(time(NULL)); 
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