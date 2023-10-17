#include "libcodehappy.h"

int main(void) {
	{
	SaturatedU8 v(110);

	std::cout << "u8 start   : " << int(v) << std::endl;	
	v += v;
	std::cout << "plus itself: " << int(v) << std::endl;
	v += v;
	std::cout << "again      : " << int(v) << std::endl;
	v -= 100;
	std::cout << "minus 100  : " << int(v) << std::endl;
	v /= 4;
	std::cout << "divideby 4 : " << int(v) << std::endl;
	v -= 100;
	std::cout << "minus 100  : " << int(v) << std::endl;
	v = 2;
	v = 100UL - v;
	std::cout << "should = 98: " << int(v) << std::endl;
	}

	{
	SaturatedU16 v(10000);

	std::cout << "u16 start   : " << int(v) << std::endl;	
	v *= v;
	std::cout << "times itself: " << int(v) << std::endl;
	v /= 12;
	std::cout << "divided 12  : " << int(v) << std::endl;
	v -= SaturatedU16(5000);
	std::cout << "minus 5000  : " << int(v) << std::endl;
	v -= 5000UL;
	std::cout << "again       : " << int(v) << std::endl;
	}

	{
	SaturatedI32 v(-10);

	for (int e = 0; e < 11; ++e) {
		v *= -10;
		std::cout << "v value (i32): " << int(v) << std::endl;
	}
	}

	{
	SaturatedI64 v(-100);
	for (int e = 0; e < 11; ++e) {
		v *= -100;
		std::cout << "v value (i64): " << i64(v) << std::endl;
	}
	}

	return 0;
}
