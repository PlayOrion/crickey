/*
 ============================================================================
 Name        : crickey.c
 Author      : KK
 Version     : v1.0
 Copyright   : copy the left
 Description : 	 we wanted a self-understood, preferably self-implemented crc code, that we can use freely.
 	 	 	 	 after a lot of hand-wringing and hair pulling, we finally understood the general
 	 	 	 	 class of CRC algorithms for various widths, and the implementation level tricks as well.
 	 	 	 	 >> theory, background: http://www.zlib.net/crc_v3.txt (Ross Williams)
 	 	 	 	 >> practical, implementation oriented: http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html (Bastian Molkenthin)
 	 	 	 	 	 check out the references section specially.
 	 	 	 	 >> handbook/reference for current CRC algos: http://reveng.sourceforge.net/crc-catalogue/ (Greg Cook)
 	 	 	 	 Thanks to these guys, I finally have a working and clearly understood crc implementation.
 	 	 	 	 the goal is to target any crc-8/crc-16/crc-32 algorithms out there, leaving out the exotic crc widths (12/24/31...)
 	 	 	 	 and to ensure that all the common options can be changed on the go, to keep it flexible.
 	 	 	 	 this is all public domain code, so enjoy.
 ============================================================================
 */



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


///////////////////////////////////////////////////////////////////		CRC8 DEFINES ++	///////////////////////////////////////////////////////////////////
#define ENABLE_CRC_8
#define USE_TABLE_FOR_CRC_8

#ifdef USE_TABLE_FOR_CRC_8
uint8_t crc8_table [256] = {0};
#endif // #define USE_TABLE_FOR_CRC_8
///////////////////////////////////////////////////////////////////		CRC8 DEFINES --	///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////		CRC16 DEFINES ++	///////////////////////////////////////////////////////////////////
#define ENABLE_CRC_16
#define USE_TABLE_FOR_CRC_16

#ifdef USE_TABLE_FOR_CRC_16
uint16_t crc16_table [256] = {0};
#endif // #ifdef USE_TABLE_FOR_CRC_16
///////////////////////////////////////////////////////////////////		CRC16 DEFINES --	///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////		CRC32 DEFINES ++	///////////////////////////////////////////////////////////////////
#define ENABLE_CRC_32
#define USE_TABLE_FOR_CRC_32

#ifdef USE_TABLE_FOR_CRC_32
uint32_t crc32_table [256] = {0};
#endif // #ifdef USE_TABLE_FOR_CRC_32
///////////////////////////////////////////////////////////////////		CRC32 DEFINES --	///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////		COMMON DEFINES ++	///////////////////////////////////////////////////////////////////
#define TOPBIT(width)	 		(1 << (width-1))

// this portable mask stuff was stolen from: http://www.zlib.net/crc_v3.txt, Ross Williams, avoid for e.g << 32 on a 32-bit machine !!
// the original, naive stuff is commented below, followed by the magic way to do it right.
//#define CRC_MASK(width)			((1 << (width)) - 1)
#define CRC_MASK(width)			((((1L<<(width-1))-1L)<<1)|1L)

#define CRC_WIDTH_8				(8)
#define CRC_WIDTH_16			(16)
#define CRC_WIDTH_32			(32)


#define BITMASK(X) (1L << (X))
///////////////////////////////////////////////////////////////////		COMMON DEFINES --	///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////		COMMON FUNCTIONS ++		///////////////////////////////////////////////////////////////////
// stolen from:  http://www.zlib.net/crc_v3.txt, Ross Williams.
// Returns the value with the bottom b [0,32] bits reflected.
// Example: reflect(0x3e23L,3) == 0x3e26
uint32_t reflect(uint32_t value,uint8_t num_bits_to_reflect)
{
	int bit_index;
	uint32_t temp_value = value;
	for (bit_index = 0; bit_index < num_bits_to_reflect; bit_index++)
	{
		if (temp_value & (uint32_t)1)
			value |=  BITMASK((num_bits_to_reflect-1)-bit_index);
		else
			value &= ~BITMASK((num_bits_to_reflect-1)-bit_index);

		temp_value >>= 1;
	}

	return value;
}
///////////////////////////////////////////////////////////////////		COMMON FUNCTIONS --		///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////		CRC8 FUNCTIONS ++	///////////////////////////////////////////////////////////////////
#ifdef USE_TABLE_FOR_CRC_8
void generate_crc8_table(uint8_t generator_polynomial)
{
	uint16_t byte_value;
	uint8_t bit_index;
	uint8_t crc_value;

	// iterate over all byte values 0 - 255
	for (byte_value = 0; byte_value < 256; byte_value++)
	{
		// calculate the CRC-8 value for current byte

		crc_value = byte_value << (CRC_WIDTH_8 - 8); // move byte into MSB of 8Bit CRC, no-op

		for (bit_index = 0; bit_index < 8; bit_index++)
		{
			if ((crc_value & TOPBIT(CRC_WIDTH_8)) != 0)
			{
				crc_value = (crc_value << 1) ^ generator_polynomial;
			}
			else
			{
				crc_value <<= 1;
			}
		}
		/* store CRC value in lookup table */
		crc8_table[byte_value] = crc_value;

	}
}
#endif // #ifdef USE_TABLE_FOR_CRC_8


#ifdef USE_TABLE_FOR_CRC_8
void print_crc8_table()
{
	uint16_t byte_value;

	printf("crc8_table == \n{\n");

	// iterate over all byte values 0 - 255
	for (byte_value = 0; byte_value < 256; byte_value++)
	{
		printf("%02X,\n",crc8_table[byte_value]);
	}

	printf("};\n\n");
}
#endif // #ifdef USE_TABLE_FOR_CRC_8


uint8_t calculate_crc8(	uint8_t* byte_data,
						int32_t data_len,
						uint8_t crc_config_initial_value,
						uint8_t crc_config_final_xor_value,
						uint8_t crc_config_polynomial,
						uint8_t crc_config_reflect_input,
						uint8_t crc_config_reflect_output )
{
	int32_t byte_data_index, bit_index;

	uint8_t calculated_crc = crc_config_initial_value ;

	for(byte_data_index = 0; byte_data_index < data_len; byte_data_index++) // for each byte of data:
	{
		// xor in the next input byte, at the MSB.
		calculated_crc ^= ( byte_data[byte_data_index] << (CRC_WIDTH_8 - 8) );

#ifdef USE_TABLE_FOR_CRC_8

		// look up the precalc CRC from table:
		calculated_crc = (crc8_table[calculated_crc]);

#else

		// calculate crc by iterating over each bit of current byte and applying polynomial.
		for (bit_index = 0; bit_index < 8; bit_index++)
		{
			// if the MSbit is 1, left-shift and apply the polynomial, else just left-shift
			if ((calculated_crc & TOPBIT(CRC_WIDTH_8)) != 0)
			{
				calculated_crc = ( ( (calculated_crc << 1) ^ crc_config_polynomial ) & CRC_MASK(CRC_WIDTH_8) );
			}
			else
			{
				calculated_crc <<= 1;
			}
		}
#endif // USE_TABLE_FOR_CRC_8

		// at this point, we have the calculated crc upto the current byte.
	}


	// xor with final_xor_value:
	if(crc_config_reflect_output == 1)
	{
		calculated_crc = ~calculated_crc ^ crc_config_final_xor_value;
	}
	else
	{
		calculated_crc ^= crc_config_final_xor_value;
	}

//	printf("calculated_crc_8 = 0x%02X\n",calculated_crc);

	return calculated_crc;
}


int check_crc_8_algo()
{

	// simple 1
	// CRC-8 : width=8 poly=0x07 init=0x00 refin=false refout=false xorout=0x00 check=0xf4 name="CRC-8"
#ifdef USE_TABLE_FOR_CRC_8
	generate_crc8_table(0x07); // required if testing table based implementation.
#endif // USE_TABLE_FOR_CRC_8
	if( 0xF4 != calculate_crc8((uint8_t*)"123456789",9,0x00,0x00,0x07,0,0) )
	{
		printf ("CRC-8 reference check failed!\n\n");
		return -1;
	}
	else
	{
//		printf ("CRC-8 check passed.\n\n");
	}



	// simple 2
	// CRC-8/CDMA2000 : width=8 poly=0x9b init=0xff refin=false refout=false xorout=0x00 check=0xda name="CRC-8/CDMA2000"
#ifdef USE_TABLE_FOR_CRC_8
	generate_crc8_table(0x9B); // required if testing table based implementation.
#endif // USE_TABLE_FOR_CRC_8
	if( 0xDA != calculate_crc8((uint8_t*)"123456789",9,0xFF,0x00,0x9B,0,0) )
	{
		printf ("CRC-8/CDMA2000 reference check failed!\n\n");
		return -1;
	}
	else
	{
//		printf ("CRC-8/CDMA2000 check passed.\n\n");
	}


	// additional: xor-out value
	// CRC-8/ITU : width=8 poly=0x07 init=0x00 refin=false refout=false xorout=0x55 check=0xa1 name="CRC-8/ITU"
#ifdef USE_TABLE_FOR_CRC_8
	generate_crc8_table(0x07); // required if testing table based implementation.
#endif // USE_TABLE_FOR_CRC_8
	if( 0xA1 != calculate_crc8((uint8_t*)"123456789",9,0x00,0x55,0x07,0,0) )
	{
		printf ("CRC-8/ITU reference check failed!\n\n");
		return -1;
	}
	else
	{
//		printf ("CRC-8/ITU check passed.\n\n");
	}


	// additional: refin is true. reflect input data.
	//	calculate_crc8((uint8_t*)"123456789",9,0x00,0x00,0x07,0,0,0); // CRC-8/DARC : width=8 poly=0x39 init=0x00 refin=true refout=true xorout=0x00 check=0x15 name="CRC-8/DARC"

	return 1; // ok.
}
///////////////////////////////////////////////////////////////////		CRC8 FUNCTIONS --	///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////		CRC16 FUNCTIONS ++	///////////////////////////////////////////////////////////////////
#ifdef USE_TABLE_FOR_CRC_16
void generate_crc16_table(uint16_t generator_polynomial)
{
	uint16_t byte_value;
	uint8_t bit_index;
	uint16_t crc_value;

	// iterate over all byte values 0 - 255
	for (byte_value = 0; byte_value < 256; byte_value++)
	{
		// calculate the CRC-8 value for current byte

		crc_value = byte_value << (CRC_WIDTH_16 - 8); // move byte into MSB of 16Bit CRC

		for (bit_index = 0; bit_index < 8; bit_index++)
		{
			if ((crc_value & TOPBIT(CRC_WIDTH_16)) != 0)
			{
				crc_value = (crc_value << 1) ^ generator_polynomial;
			}
			else
			{
				crc_value <<= 1;
			}
		}
		/* store CRC value in lookup table */
		crc16_table[byte_value] = crc_value;
	}
}
#endif // #ifdef USE_TABLE_FOR_CRC_16


#ifdef USE_TABLE_FOR_CRC_16
void print_crc16_table()
{
	uint16_t byte_value;

	printf("crc16_table == \n{\n");

	// iterate over all byte values 0 - 255
	for (byte_value = 0; byte_value < 256; byte_value++)
	{
		printf("%04X,\n",crc16_table[byte_value]);
	}

	printf("};\n\n");
}
#endif // #ifdef USE_TABLE_FOR_CRC_16


uint16_t calculate_crc16(	uint8_t* byte_data,
							int32_t data_len,
							uint16_t crc_config_initial_value,
							uint16_t crc_config_final_xor_value,
							uint16_t crc_config_polynomial,
							uint8_t crc_config_reflect_input,
							uint8_t crc_config_reflect_output )
{
	int32_t byte_data_index, bit_index;

	uint16_t calculated_crc = crc_config_initial_value;

	for(byte_data_index = 0; byte_data_index < data_len; byte_data_index++) // for each byte of data:
	{
		// xor in the next input byte, **at the MSB**
		if(crc_config_reflect_input == 1)
		{
//			printf("data: 0x%02X, reflected: 0x%02X\n",byte_data[byte_data_index], reflect(byte_data[byte_data_index],8));
			calculated_crc ^= reflect(byte_data[byte_data_index],8) << (CRC_WIDTH_16-8);
		}
		else
		{
			calculated_crc ^= byte_data[byte_data_index] << (CRC_WIDTH_16-8);
		}

#ifdef USE_TABLE_FOR_CRC_16

		// http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
		// (1)The important point is here that after xoring the current byte into the MSB of the intermediate CRC,
		// the MSB is the index into the lookup table, so take ONLY MSB for lookup table index... this explains the crc_value >> (WIDTH-8)
		// used for the lookup table.
		// (2) now, as we are getting the value corresponding to MSB from the lookup table, drop the MSB from the crc
		// i.e. shift the crc left, dropping the msb, then XOR this crc/remainder with the lookuptable value.
		calculated_crc = (calculated_crc << 8) ^ (crc16_table[calculated_crc >> (CRC_WIDTH_16-8) ]);

#else

		// calculate crc by iterating over each bit of current byte and applying polynomial.
		for (bit_index = 0; bit_index < 8; bit_index++)
		{
			// if the MSbit is 1, left-shift and apply the polynomial, else just left-shift
			if ((calculated_crc & TOPBIT(CRC_WIDTH_16)) != 0)
			{
				calculated_crc = ( ( (calculated_crc << 1) ^ crc_config_polynomial ) & CRC_MASK(CRC_WIDTH_16) );
			}
			else
			{
				calculated_crc <<= 1;
			}
		}

#endif // USE_TABLE_FOR_CRC_16

		// at this point, we have the calculated crc upto the current byte.
	}

	// xor with final_xor_value:
	if(crc_config_reflect_output == 1)
	{
		calculated_crc = (reflect(calculated_crc,CRC_WIDTH_16) & CRC_MASK(CRC_WIDTH_16)) ^ crc_config_final_xor_value;
	}
	else
	{
		calculated_crc ^= crc_config_final_xor_value;
	}

//	printf("calculated_crc_16 = 0x%04X\n",calculated_crc);

	return calculated_crc;
}


int check_crc_16_algo()
{
	// simple 1
	// CRC-16/AUG-CCITT : width=16 poly=0x1021 init=0x1d0f refin=false refout=false xorout=0x0000 check=0xe5cc name="CRC-16/AUG-CCITT"
#ifdef USE_TABLE_FOR_CRC_16
	generate_crc16_table(0x1021); // required if testing table based implementation.
#endif // USE_TABLE_FOR_CRC_8
	if( 0xE5CC != calculate_crc16((uint8_t*)"123456789",9,0x1D0F,0x0000,0x1021,0,0) )
	{
		printf ("CRC-16/AUG-CCITT reference check failed!\n\n");
		return -1;
	}
	else
	{
//		printf ("CRC-16/AUG-CCITT check passed.\n\n");
	}


	// simple 2
	// CRC-16/CCITT-FALSE: width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1 name="CRC-16/CCITT-FALSE"
#ifdef USE_TABLE_FOR_CRC_16
	generate_crc16_table(0x1021); // required if testing table based implementation.
#endif // USE_TABLE_FOR_CRC_8
	if( 0x29B1 != calculate_crc16((uint8_t*)"123456789",9,0xFFFF,0x0000,0x1021,0,0) )
	{
		printf ("CRC-16/CCITT-FALSE reference check failed!\n\n");
		return -1;
	}
	else
	{
//		printf ("CRC-16/CCITT-FALSE check passed.\n\n");
	}


	// reflect input and reflect crc
	// ARC : width=16 poly=0x8005 init=0x0000 refin=true refout=true xorout=0x0000 check=0xbb3d name="ARC"
#ifdef USE_TABLE_FOR_CRC_16
	generate_crc16_table(0x8005); // required if testing table based implementation.
#endif // USE_TABLE_FOR_CRC_8
	if( 0xBB3D != calculate_crc16((uint8_t*)"123456789",9,0x0000,0x0000,0x8005,1,1) )
	{
		printf ("ARC reference check failed!\n\n");
		return -1;
	}
	else
	{
//		printf ("ARC check passed.\n\n");
	}


	return 1; // ok.
}
///////////////////////////////////////////////////////////////////		CRC16 FUNCTIONS --	///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////		CRC32 FUNCTIONS ++	///////////////////////////////////////////////////////////////////
#ifdef USE_TABLE_FOR_CRC_32
void generate_crc32_table(uint32_t generator_polynomial)
{
	uint16_t byte_value;
	uint8_t bit_index;
	uint32_t crc_value;

	// iterate over all byte values 0 - 255
	for (byte_value = 0; byte_value < 256; byte_value++)
	{
		// calculate the CRC-8 value for current byte

		crc_value = byte_value << (CRC_WIDTH_32 - 8); // move byte into MSB of 32Bit CRC

		for (bit_index = 0; bit_index < 8; bit_index++)
		{
			if ((crc_value & TOPBIT(CRC_WIDTH_32)) != 0)
			{
				crc_value = (crc_value << 1) ^ generator_polynomial;
			}
			else
			{
				crc_value <<= 1;
			}
		}
		/* store CRC value in lookup table */
		crc32_table[byte_value] = crc_value;
	}
}
#endif // #ifdef USE_TABLE_FOR_CRC_32


#ifdef USE_TABLE_FOR_CRC_32
void print_crc32_table()
{
	uint16_t byte_value;

	printf("crc32_table == \n{\n");

	// iterate over all byte values 0 - 255
	for (byte_value = 0; byte_value < 256; byte_value++)
	{
		printf("%08X,\n",crc32_table[byte_value]);
	}

	printf("};\n\n");
}
#endif // #ifdef USE_TABLE_FOR_CRC_32


uint32_t calculate_crc32(	uint8_t* byte_data,
							int32_t data_len,
							uint32_t crc_config_initial_value,
							uint32_t crc_config_final_xor_value,
							uint32_t crc_config_polynomial,
							uint8_t crc_config_reflect_input,
							uint8_t crc_config_reflect_output )
{
	int32_t byte_data_index, bit_index;

	uint32_t calculated_crc = crc_config_initial_value;

	for(byte_data_index = 0; byte_data_index < data_len; byte_data_index++) // for each byte of data:
	{
		// xor in the next input byte, **at the MSB**
		if(crc_config_reflect_input == 1)
		{
			//			printf("data: 0x%02X, reflected: 0x%02X\n",byte_data[byte_data_index], reflect(byte_data[byte_data_index],8));
			calculated_crc ^= reflect(byte_data[byte_data_index],8) << (CRC_WIDTH_32-8);
		}
		else
		{
			calculated_crc ^= byte_data[byte_data_index] << (CRC_WIDTH_32-8);
		}

#ifdef USE_TABLE_FOR_CRC_32

		// http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
		// (1)The important point is here that after xoring the current byte into the MSB of the intermediate CRC,
		// the MSB is the index into the lookup table, so take ONLY MSB for lookup table index... this explains the crc_value >> (WIDTH-8)
		// used for the lookup table.
		// (2) now, as we are getting the value corresponding to MSB from the lookup table, drop the MSB from the crc
		// i.e. shift the crc left, dropping the msb, then XOR this crc/remainder with the lookuptable value.
		calculated_crc = (calculated_crc << 8) ^ (crc32_table[calculated_crc >> (CRC_WIDTH_32-8) ]);

#else


		// calculate crc by iterating over each bit of current byte and applying polynomial.
		for (bit_index = 0; bit_index < 8; bit_index++)
		{
			// if the MSbit is 1, left-shift and apply the polynomial, else just left-shift
			if ((calculated_crc & TOPBIT(CRC_WIDTH_32)) != 0)
			{
				calculated_crc = ( ( (calculated_crc << 1) ^ crc_config_polynomial ) & CRC_MASK(CRC_WIDTH_32) );
			}
			else
			{
				calculated_crc <<= 1;
			}
		}


#endif // USE_TABLE_FOR_CRC_32


		// at this point, we have the calculated crc upto the current byte.
	}

	// xor with final_xor_value:
	if(crc_config_reflect_output == 1)
	{
		calculated_crc =  (reflect(calculated_crc,CRC_WIDTH_32) & CRC_MASK(CRC_WIDTH_32)) ^ crc_config_final_xor_value;
	}
	else
	{
		calculated_crc ^= crc_config_final_xor_value;
	}

//	printf("calculated_crc_32 = 0x%04X\n",calculated_crc);

	return calculated_crc;
}


int check_crc_32_algo()
{
	// simple 1
	// CRC-32/BZIP2 : width=32 poly=0x04c11db7 init=0xffffffff refin=false refout=false xorout=0xffffffff check=0xfc891918 name="CRC-32/BZIP2"
#ifdef USE_TABLE_FOR_CRC_32
	generate_crc32_table(0x04C11DB7); // required if testing table based implementation.
#endif // USE_TABLE_FOR_CRC_8
	if( 0xFC891918 != calculate_crc32((uint8_t*)"123456789",9,0xFFFFFFFF,0xFFFFFFFF,0x04C11DB7,0,0) )
	{
		printf ("CRC-32/BZIP2 reference check failed!\n\n");
		return -1;
	}
	else
	{
//		printf ("CRC-32/BZIP2 check passed.\n\n");
	}



	// reflect input data and output crc
	// CRC-32 : width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926 name="CRC-32"
#ifdef USE_TABLE_FOR_CRC_32
	generate_crc32_table(0x04C11DB7); // required if testing table based implementation.
#endif // USE_TABLE_FOR_CRC_8
	if( 0xCBF43926 != calculate_crc32((uint8_t*)"123456789",9,0xFFFFFFFF,0xFFFFFFFF,0x04C11DB7,1,1) )
	{
		printf ("CRC-32 reference check failed!\n\n");
		return -1;
	}
	else
	{
//		printf ("CRC-32 check passed.\n\n");
	}



	// reflect input data and output crc - 2
	// CRC-32C : width=32 poly=0x1edc6f41 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xe3069283 name="CRC-32C"
#ifdef USE_TABLE_FOR_CRC_32
	generate_crc32_table(0x1EDC6f41); // required if testing table based implementation.
#endif // USE_TABLE_FOR_CRC_8
	if( 0xE3069283 != calculate_crc32((uint8_t*)"123456789",9,0xFFFFFFFF,0xFFFFFFFF,0x1EDC6f41,1,1) )
	{
		printf ("CRC-32C reference check failed!\n\n");
		return -1;
	}
	else
	{
//		printf ("CRC-32C check passed.\n\n");
	}



	return 1; // ok.
}
///////////////////////////////////////////////////////////////////		CRC32 FUNCTIONS --	///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////		MAIN ++		///////////////////////////////////////////////////////////////////
int main(void)
{
	puts("crickey!"); // prints crickey!


	// self-check:
	if( check_crc_8_algo() == 1)
	{
		printf("\n\nCRC-8 looks good.\n");
	}

	if( check_crc_16_algo() == 1)
	{
		printf("\n\nCRC-16 looks good.\n");
	}

	if( check_crc_32_algo() == 1)
	{
		printf("\n\nCRC-32 looks good.\n");
	}


//	generate_crc8_table(0x1D);
//	print_crc8_table();

//	generate_crc16_table(0x1189);
//	print_crc16_table();

//	generate_crc32_table(0x1189);
//	print_crc32_table();



	return EXIT_SUCCESS;
}
///////////////////////////////////////////////////////////////////		MAIN --		///////////////////////////////////////////////////////////////////
