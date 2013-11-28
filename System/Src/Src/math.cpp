#include "math.h"

float sqrt(float x){
	asm volatile("\tfld	%0\n": : "m" (x));
	asm volatile("\tfsqrt\n");
	asm volatile("\tfst	%0\n": : "m" (x));

	return x;
}

float abs(float value){
	if (value < 0)
		return value * (-1);
	else
		return value;
}

unsigned int abs(int value){
	if (value < 0)
		return value * (-1);
	else
		return value;
}

float cos(float value){
	asm volatile("\tfld	%0\n": : "m" (value));
	asm volatile("\tfcos\n");
	asm volatile("\tfst	%0\n": : "m" (value));

	return value;
};

float sin(float value){
	asm volatile("\tfld	%0\n": : "m" (value));
	asm volatile("\tfsin\n");
	asm volatile("\tfst	%0\n": : "m" (value));

	return value;
};

float asin(float value){
	asm volatile("\tfld	%0\n": : "m" (value));
	asm volatile("\tfld	%st(0)\n");
	asm volatile("\tfmul	%st, %st(0)\n");
	asm volatile("\tfsubr\n");
	asm volatile("\tfsqrt\n");
	asm volatile("\tfpatan\n");
	asm volatile("\tfst	%0\n": : "m" (value));

	return value;
};

float tan(float value){
	float vSin = sin(value);
	float vCos = cos(value);
	
	if (vCos == 0)
		return 0.0f;
	else
		return vSin / vCos;
};

extern "C"{
    int _ftol2(float f){
        volatile int result = 0;
        asm volatile("\tfistp	%0\n": : "m" (result));

        return result;
    }

    int _ftol2_sse(float f){
        volatile int result = 0;
        asm volatile("\tfistp	%0\n": : "m" (result));

        return result;
    }
}
