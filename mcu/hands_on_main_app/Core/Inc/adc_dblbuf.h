#ifndef INC_ADC_DBLBUF_H_
#define INC_ADC_DBLBUF_H_

#include "main.h"
#include "config.h"
#include "arm_math.h"

// ADC parameters
#define ADC_BUF_SIZE SAMPLES_PER_MELVEC


int StartADCAcq(int32_t n_bufs);
int IsADCFinished(void);
void encode_packet(uint8_t *packet, uint32_t* packet_cnt);
void print_encoded_packet(uint8_t *packet);

extern ADC_HandleTypeDef hadc1;

#endif /* INC_ADC_DBLBUF_H_ */
