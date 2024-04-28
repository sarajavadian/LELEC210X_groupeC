/*
 * spectrogram.c
 *
 *  Created on: Jun 4, 2021
 *      Author: math
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "spectrogram.h"
#include "spectrogram_tables.h"
#include "config.h"
#include "utils.h"
#include "arm_absmax_q15.h"

q15_t buf    [  SAMPLES_PER_MELVEC  ]; // Windowed samples
q15_t buf_fft[2*SAMPLES_PER_MELVEC  ]; // Double size (real|imag) buffer needed for arm_rfft_q15
q15_t buf_tmp[  SAMPLES_PER_MELVEC/2]; // Intermediate buffer for arm_mat_mult_fast_q15
q15_t buf_test    [  SAMPLES_PER_MELVEC  ]; // Windowed samples -> testing buffer
q15_t buf_test_2    [  SAMPLES_PER_MELVEC  ]; // Windowed samples -> testing buffer

// Convert 12-bit DC ADC samples to Q1.15 fixed point signal and remove DC component
void Spectrogram_Format(q15_t *buf)
{
	// STEP 0.1 : Increase fixed-point scale
	//            --> Pointwise shift
	//            Complexity: O(N)
	//            Number of cycles: <TODO>

	// The output of the ADC is stored in an unsigned 12-bit format, so buf[i] is in [0 , 2**12 - 1]
	// In order to better use the scale of the signed 16-bit format (1 bit of sign and 15 integer bits), we can multiply by 2**(15-12) = 2**3
	// That way, the value of buf[i] is in [0 , 2**15 - 1]

	// /!\ When multiplying/dividing by a power 2, always prefer shifting left/right instead, ARM instructions to do so are more efficient.
	// Here we should shift left by 3.
	//start_cycle_count();
	arm_shift_q15(buf, 3, buf, SAMPLES_PER_MELVEC);
	//stop_cycle_count("Fixed point scale");
	// STEP 0.2 : Remove DC Component
	//            --> Pointwise subtract
	//            Complexity: O(N)
	//            Number of cycles: <TODO>

	// Since we use a signed representation, we should now center the value around zero, we can do this by subtracting 2**14.
	// Now the value of buf[i] is in [-2**14 , 2**14 - 1]

	// can create a vector full of constant number
	// or (better) create an arm function changing slightly the subtract already implemented but where ?

// -------------------------------TEST--------------------------------------
// test to check if the loop does the same as the new line -> this is confirmed !
//	for (uint16_t i=0; i< SAMPLES_PER_MELVEC; i++){
//		buf[i] -= (1 << 14);
//	}
//
//	printf("1 << 14 en hexa : %x\n", -(1 << 14));
//	printf("-------------------ARM_OFFSET-------------------\n");
//	for (int i=0; i<SAMPLES_PER_MELVEC; i++){
//		printf("%d vs ", buf[i]);
//		printf("%d\n", buf_test[i]);
//	}
//	printf("\n-------------------ARM_OFFSET-------------------\n");

	//start_cycle_count();
	arm_offset_q15(buf, -(1 << 14), buf, SAMPLES_PER_MELVEC);
	//stop_cycle_count("Remove DC");
}

// Compute spectrogram of samples and transform into MEL vectors.
void Spectrogram_Compute(q15_t *samples, q15_t *melvec)
{
	// STEP 1  : Windowing of input samples
	//           --> Pointwise product
	//           Complexity: O(N)
	//           Number of cycles: <TODO>
	//start_cycle_count();

	arm_mult_q15(samples, hamming_window, buf, SAMPLES_PER_MELVEC);

	//stop_cycle_count("Windowing");

	// STEP 2  : Discrete Fourier Transform
	//           --> In-place Fast Fourier Transform (FFT) on a real signal
	//           --> For our spectrogram, we only keep only positive frequencies (symmetry) in the next operations.
	//           Complexity: O(Nlog(N))
	//           Number of cycles: <TODO>

	// Since the FFT is a recursive algorithm, the values are rescaled in the function to ensure that overflow cannot happen.
	//start_cycle_count();

	arm_rfft_instance_q15 rfft_inst;

	arm_rfft_init_q15(&rfft_inst, SAMPLES_PER_MELVEC, 0, 1);

	arm_rfft_q15(&rfft_inst, buf, buf_fft);
	//stop_cycle_count("FFT");

	// STEP 3  : Compute the complex magnitude of the FFT
	//           Because the FFT can output a great proportion of very small values,
	//           we should rescale all values by their maximum to avoid loss of precision when computing the complex magnitude
	//           In this implementation, we use integer division and multiplication to rescale values, which are very costly.

	// STEP 3.1: Find the extremum value (maximum of absolute values)
	//           Complexity: O(N)
	//           Number of cycles: <TODO>
	//start_cycle_count();

	q15_t vmax;
	uint32_t pIndex=0;

	arm_absmax_q15(buf_fft, SAMPLES_PER_MELVEC, &vmax, &pIndex); // is there a way to go faster ? Maybe directly output the log2 of vmax such that the loop below is not necessary
	//stop_cycle_count("extremum");

	// -> also check if this is really relevant to multiple by the max, precision-wise. If not much difference is seen, we could simplify the code by removing this part.
	// STEP 3.2: Normalize the vector - Dynamic range increase
	//           Complexity: O(N)
	//           Number of cycles: <TODO>

	// code to get the highest power of 2 contained in vmax (to approximate vmax) (ex : 70 -> 64, 2047 -> 1024)
	//start_cycle_count();
	int vmax_round = -1;
	for (int i=14; vmax_round == -1; i--){
		if ((vmax >> i) & 1){
			vmax_round = i;
		}
	}
//
	arm_shift_q15(buf_fft, 15-vmax_round-1, buf, SAMPLES_PER_MELVEC); // There can be overflow so maybe -1 in the shift. But in that case, the approximation is way off.
	//stop_cycle_count("Normalize");

//	------------------ TEST ------------------------
//	for (int i=0; i < SAMPLES_PER_MELVEC; i++) // We don't use the second half of the symmetric spectrum
//	{
//		buf[i] = (q15_t) (((q31_t) buf_fft[i] << 15) /((q31_t)vmax)); // better to find the closest power of 2 and shift instead.
//	}
//	printf("vmax : %d\n", vmax);
//	printf("vmax_round : %d", vmax_round);
//	printf("vmax_approx : %f\n", pow(2, vmax_round));
//	printf("-------------------SCALE_DOWN-------------------\n");
//		for (int i=0; i<SAMPLES_PER_MELVEC; i++){
//			printf("from : %d\n", buf_fft[i]);
//			printf("%d vs ", buf[i]);
//			printf("%d\n", buf_test[i]);
//		}
//	printf("\n-------------------SCALE_DOWN-------------------\n");

	//stop_cycle_count("3.2");

	// STEP 3.3: Compute the complex magnitude
	//           --> The output buffer is now two times smaller because (real|imag) --> (mag)
	//           Complexity: O(N)
	//           Number of cycles: <TODO>
	//start_cycle_count();

//	memcpy(buf_test, buf, SAMPLES_PER_MELVEC);
//	printf("---------------BEFORE---------------\n");
//	for (int i=0; i<40;i++){
//		printf("%d|", buf[i]);
//	}
//	printf("\n----------------------------------------\n");
//	start_cycle_count();
	arm_cmplx_mag_q15(buf, buf, SAMPLES_PER_MELVEC/2); // could completely change the function by making approximations. It could save some clock cycles
//	stop_cycle_count("complex mag\n");
//	printf("---------------AFTER---------------\n");
////	for (int i=0; i<20; i++){
////		printf("%d|", buf_test_2[i]);
////	}
////	printf("\n---------------END---------------\n");
//	start_cycle_count();
//	uint32_t blkCnt;                               /* Loop counter */
//
//	q15_t real, imag;                              /* Temporary input variables */
//
//	q15_t* pSrc = buf_test;
//	q15_t* pDst = buf_test;
//	/* Accumulators */
//
//  /* Initialize blkCnt with number of samples */
//  blkCnt = SAMPLES_PER_MELVEC/2;
//  while (blkCnt > 0U)
//  {
////	  printf("good value : %d\n", buf[i]);
//	  real = *pSrc++;
//	  imag = *pSrc++;
//	  real = real > 0? real : (q15_t)__QSUB16(0, real);
//	  imag = imag > 0? imag : (q15_t)__QSUB16(0, imag);
////	  printf("real value : %d\n", real);
////	  printf("imag value : %d\n", imag);
//
//	/* store result in 2.14 format in destination buffer. */
//	*pDst = (q15_t)__QSUB16(real, imag) > 0 ? real + (imag>>2) : imag + (real>>2);
////	printf("approx value : %d\n", *pDst);
//	pDst++;
////	i++;
//	// other question to ask : if shift right, should we remove the sign bit before and add it after ?
//	/* Decrement loop counter */
//	blkCnt--;
//  }
//	stop_cycle_count("New complex mag");
//	printf("---------------AFTER MY FUNCTION---------------\n");
//	for (int i=0; i<20; i++){
//		printf("%d|", buf_test[i]);
//	}
//	printf("\n---------------END---------------\n");

	//stop_cycle_count("cmplx mag");

	// STEP 3.4: Denormalize the vector
	//           Complexity: O(N)
	//           Number of cycles: <TODO>

	//start_cycle_count();

//	--------------------------------TEST-----------------------------------------
//	memcpy(buf_test_2, buf, SAMPLES_PER_MELVEC); // just for the test to keep the input buffer and compare it to the output
//	arm_shift_q15(buf, vmax_round-15, buf_test, SAMPLES_PER_MELVEC/2);
//	for (int i=0; i < SAMPLES_PER_MELVEC/2; i++)
//	{
//		buf[i] = (q15_t) ((((q31_t) buf[i]) * ((q31_t) vmax) ) >> 15 );
//	}
//	printf("-------------------SCALE_UP-------------------\n");
//		for (int i=0; i<SAMPLES_PER_MELVEC/2; i++){
//			printf("From : %d\n", buf_test_2[i]);
//			printf("%d vs ", buf[i]);
//			printf("%d\n", buf_test[i]);
//		}
//	printf("\n-------------------SCALE_UP-------------------\n");

	//start_cycle_count();
	arm_shift_q15(buf, vmax_round-15+1, buf, SAMPLES_PER_MELVEC/2);
	//stop_cycle_count("Denormalize");

	//stop_cycle_count("3.4");
	// STEP 4:   Apply MEL transform
	//           --> Fast Matrix Multiplication
	//           Complexity: O(Nmel*N)
	//           Number of cycles: <TODO>

	// /!\ The difference between the function arm_mat_mult_q15() and the fast variant is that the fast variant use a 32-bit rather than a 64-bit accumulator.
	// The result of each 1.15 x 1.15 multiplication is truncated to 2.30 format. These intermediate results are accumulated in a 32-bit register in 2.30 format.
	// Finally, the accumulator is saturated and converted to a 1.15 result. The fast version has the same overflow behavior as the standard version but provides
	// less precision since it discards the low 16 bits of each multiplication result.

	// /!\ In order to avoid overflows completely the input signals should be scaled down. Scale down one of the input matrices by log2(numColsA) bits to avoid overflows,
	// as a total of numColsA additions are computed internally for each output element. Because our hz2mel_mat matrix contains lots of zeros in its rows, this is not necessary.

	//start_cycle_count();
//	arm_matrix_instance_q15 hz2mel_inst, fftmag_inst, melvec_inst;
//
//	arm_mat_init_q15(&hz2mel_inst, MELVEC_LENGTH, SAMPLES_PER_MELVEC/2, hz2mel_mat); // MELVEC_LENGTH x SAMPLES_PER_MELVEC/2 = 20 x 256. This matrix is a band matrix
//	arm_mat_init_q15(&fftmag_inst, SAMPLES_PER_MELVEC/2, 1, buf); // SAMPLES_PER_MELVEC/2 x 1 = 256 x 1  // requires 21k cycles -> matrix band implementation could save us some precious cycles.
//	arm_mat_init_q15(&melvec_inst, MELVEC_LENGTH, 1, melvec); // result : MELVEC_LENGTH x 1 = 20 x 1
//
//	arm_mat_mult_fast_q15(&hz2mel_inst, &fftmag_inst, &melvec_inst, buf_tmp);
	//stop_cycle_count("Leur_calcul");

	///////////////////////// MULTIPLICATION IMPLEMENTATION /////////////////////////

	//start_cycle_count();
	q63_t result_temp[MELVEC_LENGTH];

	for (uint8_t i = 0; i < MELVEC_LENGTH; i++){  // pour chaque ligne de hz2mel_mat :
		uint8_t j = start_idx[i];
		uint8_t l = length[i];
		// dot product entre le vecteur de chaque ligne et le vecteur buf
		arm_dot_prod_q15(hz2mel_mat + i*SAMPLES_PER_MELVEC/2 + j, buf + j, l, result_temp + i); // résultat sur 64 bits

        q63_t sign = (result_temp[i] & 0x8000000000000000) >> 48; // on récupère le bit de signe et on le met au 1er bit (sur 16)
        q15_t temp = (q15_t) (result_temp[i] >> 15); // on remet le résultat sur 16 bits
        temp  = ((temp << 1) >> 1); // on supprime le premier bit (bit bullshit)
        melvec[i] = temp | (q15_t) sign; // on met le bit de signe au premier bit
	}
	//stop_cycle_count("Our_calcul");


	//printf("Buffer :\n");
	//for (int k = 0; k < SAMPLES_PER_MELVEC/2; k++){
		//printf("%d  ", buf[k]);
	//}
	//printf("\n Our_melvec: \n");
	//for (int k = 0; k < MELVEC_LENGTH; k++){
		//printf("%d  ", our_melvec[k]);
	//}
	//printf("\n Original: \n");
	//for (int k = 0; k < MELVEC_LENGTH; k++){
		//printf("%d  ", melvec[k]);
	//}
	//printf("\n");


	/////////////////////////////////////////////////////////////////////////////////


//	int temp;
//	for (int i=0; i < MELVEC_LENGTH; i++){
//		temp = 0;
//		for (int j=0; j < SAMPLES_PER_MELVEC/2; j++){
//			for (int k=0; k < SAMPLES_PER_MELVEC/2; k++){
//				temp += hz2mel_inst[i][j] * fftmag_inst[j][i];
//			}
//		}
//	}

}
