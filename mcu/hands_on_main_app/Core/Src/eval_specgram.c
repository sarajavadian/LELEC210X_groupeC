/*
 * eval_specgram.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "arm_math.h"
#include "spectrogram.h"
#include "spectrogram_tables.h"
#include "config.h"
#include "utils.h"
#include "arm_absmax_q15.h"
#include "data.h"
#include "packet.h"
#include "adc_dblbuf.h"

#define FIRST 0 // used to display one or the other spectrogram (can be both set)
#define SECOND 1

q15_t spec_buf    [  SAMPLES_PER_MELVEC  ]; // Windowed samples
q15_t fftbuf [2*SAMPLES_PER_MELVEC  ]; // Double size (real|imag) buffer needed for arm_rfft_q15
q15_t mult_buf[  SAMPLES_PER_MELVEC/2]; // Intermediate buffer for arm_mat_mult_fast_q15

q15_t melvectors_before [N_MELVECS][MELVEC_LENGTH];
q15_t melvectors_after [N_MELVECS][MELVEC_LENGTH];

static uint32_t packet_cnt;
uint8_t packet[PACKET_LENGTH];

/**
 * @brief Extracts numbers from a string separated by "_ " delimiter.
 *
 * This function extracts numbers from the input string `data_processed`, which contains
 * numbers separated by "_ " delimiter. It parses the string character by
 * character, converting sequences of digits into integer numbers and storing
 * them in the `numbers` array. The function stops when it encounters the end
 * of the string, reaches the maximum number of numbers specified by
 * `MAX_NUMBERS`, or when it cannot parse any more numbers.
 *
 * @param data_processed Pointer to the input string containing numbers separated by "_ " delimiter.
 * @param numbers Pointer to the array where extracted numbers will be stored.
 * @param count Pointer to an integer variable to store the number of numbers extracted.
 * @note The `numbers` array should have sufficient space to store the extracted numbers.
 * @note The function does not handle overflow or other error conditions during number parsing.
 * THANKS CHATGPT
 */

void extractNumbers(const char *data_processed, q15_t *numbers, int *count) {
    *count = 0;  // Initialize count to 0
    int num = 0;
    int isNumNegative = 0;

    while (*data_processed != '\0') {
        // Skip leading whitespace
        while (*data_processed == ' ') {
            data_processed++;
        }

        // Check if the character is a digit or a negative sign
        if ((*data_processed >= '0' && *data_processed <= '9') || *data_processed == '-') {
            // If it's a negative sign, mark that the number is negative
            if (*data_processed == '-') {
                isNumNegative = 1;
            } else {
                // Update the current number
                num = num * 10 + (*data_processed - '0');
            }
        } else if (*data_processed == '_' && *(data_processed + 1) == ' ') { // Delimiter found
            // Store the number in the array with appropriate sign
            numbers[*count] = (isNumNegative) ? -num : num;
            (*count)++;  // Increment count
            num = 0;  // Reset num for next number
            isNumNegative = 0; // Reset sign for next number
        }
        data_processed++;  // Move to the next character
    }
}


void eval_spectrogram(void)
{
	// PRINT data
	/////////////////////////////////////////////////////////////////
	// Version 1
	q15_t* melvec = malloc(sizeof(q15_t)* (N_MELVECS*SAMPLES_PER_MELVEC));
	int* nbr_data = malloc(sizeof(int));

	extractNumbers(data_white_noise, melvec, nbr_data); // extract the values from the test dataset

	if (*nbr_data != N_MELVECS*SAMPLES_PER_MELVEC){ // check if everything has been read
		printf("ERROR : only %d numbers extracted instead of %d", *nbr_data, N_MELVECS*SAMPLES_PER_MELVEC);
	}

	free(nbr_data);

	for (uint8_t curmelvec = 0; curmelvec < N_MELVECS; curmelvec++){

		// Shift
		arm_shift_q15(melvec + curmelvec*SAMPLES_PER_MELVEC, 3, spec_buf, SAMPLES_PER_MELVEC);

		// Remove DC-comp
		for(uint16_t i=0; i < SAMPLES_PER_MELVEC; i++) { // Remove DC component
			spec_buf[i] -= (1 << 14);
		}
		// Windowing
		arm_mult_q15(spec_buf, hamming_window, spec_buf, SAMPLES_PER_MELVEC);
//		if (curmelvec==0){
//			for (int i=0; i < 20; i++){
//				printf("%d|", (spec_buf[i]));
//			} // to print out the intermediate value
//			printf("END\n");
//		}
		//FFT
		arm_rfft_instance_q15 rfft_inst;
		arm_rfft_init_q15(&rfft_inst, SAMPLES_PER_MELVEC, 0, 1);
		arm_rfft_q15(&rfft_inst, spec_buf, fftbuf);

		// Find max
		q15_t vmax;
		uint32_t pIndex=0;
		arm_absmax_q15(fftbuf, SAMPLES_PER_MELVEC, &vmax, &pIndex);

		//Normalize
		for (int i=0; i < SAMPLES_PER_MELVEC; i++) // We don't use the second half of the symmetric spectrum
		{
			spec_buf[i] = (q15_t) (((q31_t) fftbuf[i] << 15) /((q31_t)vmax));
		}

		//Magnitude
		arm_cmplx_mag_q15(spec_buf, spec_buf, SAMPLES_PER_MELVEC/2);


		//Denormalize
		for (int i=0; i < SAMPLES_PER_MELVEC/2; i++)
		{
			spec_buf[i] = (q15_t) ((((q31_t) spec_buf[i]) * ((q31_t) vmax) ) >> 15 );
		}



		// MEL transform
		arm_matrix_instance_q15 hz2mel_inst, fftmag_inst, melvec_inst;

		arm_mat_init_q15(&hz2mel_inst, MELVEC_LENGTH, SAMPLES_PER_MELVEC/2, hz2mel_mat);
		arm_mat_init_q15(&fftmag_inst, SAMPLES_PER_MELVEC/2, 1, spec_buf);
		arm_mat_init_q15(&melvec_inst, MELVEC_LENGTH, 1, melvectors_before[curmelvec]);

		arm_mat_mult_fast_q15(&hz2mel_inst, &fftmag_inst, &melvec_inst, mult_buf);

		//////////////////////////////////////////////////////////////////////////////////////
		// Version 2

		Spectrogram_Format(melvec + curmelvec * SAMPLES_PER_MELVEC);
		Spectrogram_Compute(melvec + curmelvec * SAMPLES_PER_MELVEC, melvectors_after[curmelvec]);
	}

#if (DEBUGP == 1)
#if (FIRST)
	DEBUG_PRINT("FVs of the first spectrogram\r\n");
	for(unsigned int j=0; j < N_MELVECS; j++) {
		DEBUG_PRINT("FV #%u:\t", j+1);
		for(unsigned int i=0; i < MELVEC_LENGTH; i++) {
			DEBUG_PRINT("%.2f, ", q15_to_float(melvectors_before[j][i]));
		}
		DEBUG_PRINT("\r\n");
	}
	for (size_t i=0; i<N_MELVECS; i++) { // 20 // This is the function "encode_packet" which I copy-pasted to change "melvectors_before"
			for (size_t j=0; j<MELVEC_LENGTH; j++) { // 20
				(packet+PACKET_HEADER_LENGTH)[(i*MELVEC_LENGTH+j)*2]   = melvectors_before[i][j] >> 8;
				(packet+PACKET_HEADER_LENGTH)[(i*MELVEC_LENGTH+j)*2+1] = melvectors_before[i][j] & 0xFF;
			}
		}
		// Write header and tag into the packet.
		make_packet(packet, PAYLOAD_LENGTH, 0, packet_cnt);
		packet_cnt++;
		if (packet_cnt == 0) {
			// Should not happen as packet_cnt is 32-bit and we send at most 1 packet per second.
			DEBUG_PRINT("Packet counter overflow.\r\n");
			Error_Handler();
		}
	print_encoded_packet(packet); // the packet was solely created to call this function, which allows the plot of the spectrogram on python's end.
#endif

#if (SECOND)
	DEBUG_PRINT("FVs of the second spectrogram\r\n");
	for(unsigned int j=0; j < N_MELVECS; j++) {
		DEBUG_PRINT("FV #%u:\t", j+1);
		for(unsigned int i=0; i < MELVEC_LENGTH; i++) {
			DEBUG_PRINT("%.2f, ", q15_to_float(melvectors_after[j][i]));
		}
		DEBUG_PRINT("\r\n");
	}
	for (size_t i=0; i<N_MELVECS; i++) { // 20
			for (size_t j=0; j<MELVEC_LENGTH; j++) { // 20
				(packet+PACKET_HEADER_LENGTH)[(i*MELVEC_LENGTH+j)*2]   = melvectors_after[i][j] >> 8;
				(packet+PACKET_HEADER_LENGTH)[(i*MELVEC_LENGTH+j)*2+1] = melvectors_after[i][j] & 0xFF;
			}
		}
		// Write header and tag into the packet.
		make_packet(packet, PAYLOAD_LENGTH, 0, packet_cnt);
		packet_cnt++;
		if (packet_cnt == 0) {
			// Should not happen as packet_cnt is 32-bit and we send at most 1 packet per second.
			DEBUG_PRINT("Packet counter overflow.\r\n");
			Error_Handler();
		}
	print_encoded_packet(packet);
#endif
#endif

free(melvec);

}
