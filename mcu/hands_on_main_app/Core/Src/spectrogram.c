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
void Spectrogram_Format(q15_t *in)
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

	arm_shift_q15(in, 3, buf, SAMPLES_PER_MELVEC);

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

//	start_cycle_count();
	arm_offset_q15(buf, -(1 << 14), buf, SAMPLES_PER_MELVEC);
//	stop_cycle_count("0.2");
}

// Compute spectrogram of samples and transform into MEL vectors.
void Spectrogram_Compute(q15_t *samples, q15_t *melvec)
{
	// STEP 1  : Windowing of input samples
	//           --> Pointwise product
	//           Complexity: O(N)
	//           Number of cycles: <TODO>
	arm_mult_q15(samples, hamming_window, buf, SAMPLES_PER_MELVEC);

	// STEP 2  : Discrete Fourier Transform
	//           --> In-place Fast Fourier Transform (FFT) on a real signal
	//           --> For our spectrogram, we only keep only positive frequencies (symmetry) in the next operations.
	//           Complexity: O(Nlog(N))
	//           Number of cycles: <TODO>

	// Since the FFT is a recursive algorithm, the values are rescaled in the function to ensure that overflow cannot happen.
	arm_rfft_instance_q15 rfft_inst;

	arm_rfft_init_q15(&rfft_inst, SAMPLES_PER_MELVEC, 0, 1);

	arm_rfft_q15(&rfft_inst, buf, buf_fft);

	// STEP 3  : Compute the complex magnitude of the FFT
	//           Because the FFT can output a great proportion of very small values,
	//           we should rescale all values by their maximum to avoid loss of precision when computing the complex magnitude
	//           In this implementation, we use integer division and multiplication to rescale values, which are very costly.

	// STEP 3.1: Find the extremum value (maximum of absolute values)
	//           Complexity: O(N)
	//           Number of cycles: <TODO>

	q15_t vmax;
	uint32_t pIndex=0;

	arm_absmax_q15(buf_fft, SAMPLES_PER_MELVEC, &vmax, &pIndex); // is there a way to go faster ? Maybe directly output the log2 of vmax such that the loop below is not necessary
	// -> also check if this is really relevant to multiple by the max, precision-wise. If not much difference is seen, we could simplify the code by removing this part.
	// STEP 3.2: Normalize the vector - Dynamic range increase
	//           Complexity: O(N)
	//           Number of cycles: <TODO>

	// code to get the highest power of 2 contained in vmax (to approximate vmax) (ex : 70 -> 64, 2047 -> 1024)
//	start_cycle_count();
	int vmax_round = -1;
	for (int i=14; vmax_round == -1; i--){
		if ((vmax >> i) & 1){
			vmax_round = i;
		}
	}

	arm_shift_q15(buf_fft, 15-vmax_round, buf, SAMPLES_PER_MELVEC); // There can be overflow so maybe -1 in the shift. But in that case, the approximation is way off.
//	stop_cycle_count("3.2");

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

	arm_cmplx_mag_q15(buf, buf, SAMPLES_PER_MELVEC/2); // could completely change the function by making approximations. It could save some clock cycles

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

//	start_cycle_count();
	arm_shift_q15(buf, vmax_round-15, buf, SAMPLES_PER_MELVEC/2);
//	stop_cycle_count("3.4");

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
	
	arm_matrix_instance_q15 hz2mel_inst, fftmag_inst, melvec_inst;

	arm_mat_init_q15(&hz2mel_inst, MELVEC_LENGTH, SAMPLES_PER_MELVEC/2, hz2mel_mat); // MELVEC_LENGTH x SAMPLES_PER_MELVEC/2 = 20 x 256. This matrix is a band matrix
	arm_mat_init_q15(&fftmag_inst, SAMPLES_PER_MELVEC/2, 1, buf); // SAMPLES_PER_MELVEC/2 x 1 = 256 x 1  // requires 21k cycles -> matrix band implementation could save us some precious cycles.
	arm_mat_init_q15(&melvec_inst, MELVEC_LENGTH, 1, melvec); // result : MELVEC_LENGTH x 1 = 20 x 1

	arm_mat_mult_fast_q15(&hz2mel_inst, &fftmag_inst, &melvec_inst, buf_tmp);


	///////////////////////// MULTIPLICATION IMPLEMENTATION /////////////////////////

	//for (int i = 0; i < NB_LINE; i++){
	    //     int j = start_idx[i];
	    //     int l = length[i];

	    //     result[i] = dot_product(matrix + i * NB_COL + j, vect + j, l);
	    //     printf("%d\n", result[i]);
	    // }

	q63_t result_temp[MELVEC_LENGTH];
	q15_t our_melvec[MELVEC_LENGTH];
	q63_t one = 1;

	for (uint8_t i = 0; i < MELVEC_LENGTH; i++){
		uint8_t j = start_idx[i];
		uint8_t l = length[i];

		arm_dot_prod_q15(hz2mel_mat + i*SAMPLES_PER_MELVEC/2 + j, buf + j, l, result_temp + i);

		//our_melvec[i] = (q15_t) (((result_temp[i] >> 33) << 33) << 15);
		//our_melvec[i] = (q15_t) ( (result_temp[i]) / ((q63_t)1 << 47) );
		our_melvec[i] = ((result_temp[i]) >> 48);
        //q63_t sign = (result_temp[i] & (1 << 63)) >> 33;
		//sign = sign | our_melvec
		//our_melvec[i] = (q15_t) (result_temp[i])
	}


	// Define a scaling factor to bring q63_t into the range of q15_t : #define SCALE_FACTOR ((q63_t)1 << 47)  // 2^(63 - 16)
	// Conversion functionq15_t convert_q63_to_q15(q63_t input) {
	// Divide the q63_t input by the scaling factor and cast to q15_t
	//return (q15_t)(input / SCALE_FACTOR); }
	printf("%d \n", one);
	printf("%ld \n", one);
	printf("%X \n", one);
	printf("%lX \n", one);
	printf("%lld \n", one);


	printf("Buffer :\n");
	for (int k = 0; k < SAMPLES_PER_MELVEC/2; k++){
		printf("%d  ", buf[k]);
	}
	printf("\n Our_melvec: \n");
	for (int k = 0; k < MELVEC_LENGTH; k++){
		printf("%ld  ", result_temp[k]);
	}
	printf("\n Original: \n");
	for (int k = 0; k < MELVEC_LENGTH; k++){
		printf("%X  ", melvec[k]);
	}
	printf("\n");


	//q15_t *our_melvec = (q15_t *) calloc(MELVEC_LENGTH, sizeof(q15_t));
	//q15_t *buf_norm = (q15_t *) malloc(SAMPLES_PER_MELVEC/2 * sizeof(q15_t));

	// Normalisation de buf : on divise par log2(SAMPLES_PER_MELVEC/2) = 8 -> revient à shifter à droite de 3 bits
	//arm_shift_q15(buf, -3, buf_norm, SAMPLES_PER_MELVEC/2);

	//for (uint16_t k = 0; k < NB_ENTRIES; k++){
		//uint8_t i = i_indexes[k];
		//uint8_t j = j_indexes[k];
		//our_melvec[i] += hz2mel_mat[i * SAMPLES_PER_MELVEC/2 + j] * buf_norm[j];
	//}

	//------- PRINT ----------

	//printf("Buffer :\n");
	//for (uint8_t k = 0; k < SAMPLES_PER_MELVEC/2; k++){
		//printf("%d ", buf[k]);
	//}
	//printf("\n");

	//printf("Normalized buffer :\n");
	//for (uint8_t k = 0; k < SAMPLES_PER_MELVEC/2; k++){
		//printf("%d ", buf_norm[k]);
	//}
	//printf("\n");

	//printf("Original melvec :\n");
	//for (uint8_t k = 0; k < MELVEC_LENGTH; k++){
			//printf("%d ", melvec[k]);
	//}
	//printf("\n");

	//printf("Our melvec :\n");
	//for (uint8_t k = 0; k < MELVEC_LENGTH; k++){
			//printf("%d ", our_melvec[k]);
	//}
	//printf("\n");



	//free(our_melvec);
	//free(buf_norm);
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
