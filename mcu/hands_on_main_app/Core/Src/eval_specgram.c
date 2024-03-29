/*
 * eval_specgram.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "eval_specgram.h"
#include "config.h"
#include "main.h"
#include "spectrogram.h"

q15_t buf    [  SAMPLES_PER_MELVEC  ]; // Windowed samples
q15_t mel_vector[MELVEC_LENGTH];


FILE* raw_data;

void eval_spectrogram(void)
{
	raw_data = fopen(FILE_PATH, "r");

	// PRINT data
	/////////////////////////////////////////////////////////////////
	// Version 1

	// fread(buf, sizeof(q15_t), SAMPLES_PER_MELVEC, raw_data);
	buf = NULL; // transform data in q15_t *

	// Shift
	arm_shift_q15(buf, 3, buf, SAMPLES_PER_MELVEC);

	// Remove DC-comp
	for(uint16_t i=0; i < SAMPLES_PER_MELVEC; i++) { // Remove DC component
		buf[i] -= (1 << 14);
	}

	// Windowing
	arm_mult_q15(buf, hamming_window, buf, SAMPLES_PER_MELVEC);

	//FFT
	arm_rfft_instance_q15 rfft_inst;
	arm_rfft_init_q15(&rfft_inst, SAMPLES_PER_MELVEC, 0, 1);
	arm_rfft_q15(&rfft_inst, buf, buf_fft);

	// Find max
	q15_t vmax;
	uint32_t pIndex=0;
	arm_absmax_q15(buf_fft, SAMPLES_PER_MELVEC, &vmax, &pIndex);

	//Normalize
	for (int i=0; i < SAMPLES_PER_MELVEC; i++) // We don't use the second half of the symmetric spectrum
	{
		buf[i] = (q15_t) (((q31_t) buf_fft[i] << 15) /((q31_t)vmax));
	}


	//Magnitude
	arm_cmplx_mag_q15(buf, buf, SAMPLES_PER_MELVEC/2);


	//Denormalize
	for (int i=0; i < SAMPLES_PER_MELVEC/2; i++)
	{
		buf[i] = (q15_t) ((((q31_t) buf[i]) * ((q31_t) vmax) ) >> 15 );
	}

	// MEL transform
	arm_matrix_instance_q15 hz2mel_inst, fftmag_inst, melvec_inst;

	arm_mat_init_q15(&hz2mel_inst, MELVEC_LENGTH, SAMPLES_PER_MELVEC/2, hz2mel_mat);
	arm_mat_init_q15(&fftmag_inst, SAMPLES_PER_MELVEC/2, 1, buf);
	arm_mat_init_q15(&melvec_inst, MELVEC_LENGTH, 1, mel_vector);

	arm_mat_mult_fast_q15(&hz2mel_inst, &fftmag_inst, &melvec_inst, buf_tmp);

	//PRINT
	//////////////////////////////////////////////////////////////////////////////////////
	// Version 2

	Spectrogram_Format((q15_t *)ADCData[buf_cplt]);
	Spectrogram_Compute((q15_t *)ADCData[buf_cplt], mel_vector);

	//PRINT


}


