/*
 * eval_specgram.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "eval_specgram.h"
#include "config.h"
#include "main.h"
#include "spectrogram.h"

q15_t readlines [SAMPLES_PER_MELVEC];

q15_t spec_buf    [  SAMPLES_PER_MELVEC  ]; // Windowed samples
q15_t fftbuf [2*SAMPLES_PER_MELVEC  ]; // Double size (real|imag) buffer needed for arm_rfft_q15
q15_t mult_buf[  SAMPLES_PER_MELVEC/2]; // Intermediate buffer for arm_mat_mult_fast_q15

q15_t melvectors_before [N_MELVECS][MELVEC_LENGTH];
q15_t melvectors_after [N_MELVECS][MELVEC_LENGTH];


FILE* raw_data;

void eval_spectrogram(void)
{
//	raw_data = fopen(FILE_PATH, "r");
//
//	// PRINT data
//	/////////////////////////////////////////////////////////////////
//	// Version 1

//	for (uint8_t curmelvec = 0; curmelvec < N_MELVECS; curmelvec++){
//		fread(readlines, sizeof(q15_t), SAMPLES_PER_MELVEC, raw_data);
//		// transform data in q15_t *
//
//		// Shift
//		arm_shift_q15(readlines, 3, spec_buf, SAMPLES_PER_MELVEC);
//
//		// Remove DC-comp
//		for(uint16_t i=0; i < SAMPLES_PER_MELVEC; i++) { // Remove DC component
//			spec_buf[i] -= (1 << 14);
//		}
//
//		// Windowing
//		arm_mult_q15(spec_buf, hamming_window, spec_buf, SAMPLES_PER_MELVEC);
//
//		//FFT
//		arm_rfft_instance_q15 rfft_inst;
//		arm_rfft_init_q15(&rfft_inst, SAMPLES_PER_MELVEC, 0, 1);
//		arm_rfft_q15(&rfft_inst, spec_buf, fftbuf);
//
//		// Find max
//		q15_t vmax;
//		uint32_t pIndex=0;
//		arm_absmax_q15(fftbuf, SAMPLES_PER_MELVEC, &vmax, &pIndex);
//
//		//Normalize
//		for (int i=0; i < SAMPLES_PER_MELVEC; i++) // We don't use the second half of the symmetric spectrum
//		{
//			spec_buf[i] = (q15_t) (((q31_t) fftbuf[i] << 15) /((q31_t)vmax));
//		}
//
//
//		//Magnitude
//		arm_cmplx_mag_q15(spec_buf, spec_buf, SAMPLES_PER_MELVEC/2);
//
//
//		//Denormalize
//		for (int i=0; i < SAMPLES_PER_MELVEC/2; i++)
//		{
//			spec_buf[i] = (q15_t) ((((q31_t) spec_buf[i]) * ((q31_t) vmax) ) >> 15 );
//		}
//
//		// MEL transform
//		arm_matrix_instance_q15 hz2mel_inst, fftmag_inst, melvec_inst;
//
//		arm_mat_init_q15(&hz2mel_inst, MELVEC_LENGTH, SAMPLES_PER_MELVEC/2, hz2mel_mat);
//		arm_mat_init_q15(&fftmag_inst, SAMPLES_PER_MELVEC/2, 1, spec_buf);
//		arm_mat_init_q15(&melvec_inst, MELVEC_LENGTH, 1, melvectors_before[curmelvec]);
//
//		arm_mat_mult_fast_q15(&hz2mel_inst, &fftmag_inst, &melvec_inst, mult_buf);
//
//		//////////////////////////////////////////////////////////////////////////////////////
//		// Version 2
//
//		Spectrogram_Format(readlines);
//		Spectrogram_Compute(readlines, melvectors_after[curmelvec]);
//
//	}
//
//#if (DEBUGP == 1)
//	DEBUG_PRINT("FVs of the first spectrogram\r\n");
//	for(unsigned int j=0; j < N_MELVECS; j++) {
//		DEBUG_PRINT("FV #%u:\t", j+1);
//		for(unsigned int i=0; i < MELVEC_LENGTH; i++) {
//			DEBUG_PRINT("%.2f, ", q15_to_float(melvectors_before[j][i]));
//		}
//		DEBUG_PRINT("\r\n");
//	}
//	DEBUG_PRINT("FVs of the second spectrogram\r\n");
//	for(unsigned int j=0; j < N_MELVECS; j++) {
//		DEBUG_PRINT("FV #%u:\t", j+1);
//		for(unsigned int i=0; i < MELVEC_LENGTH; i++) {
//			DEBUG_PRINT("%.2f, ", q15_to_float(melvectors_after[j][i]));
//		}
//		DEBUG_PRINT("\r\n");
//	}
//#endif


}
