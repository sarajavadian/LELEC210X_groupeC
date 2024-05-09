/*
 * arm_cmplx_mag_fast_q15.h
 *
 *  Created on: Apr 29, 2024
 *      Author: aurel
 */

#ifndef INC_ARM_CMPLX_MAG_FAST_Q15_H_
#define INC_ARM_CMPLX_MAG_FAST_Q15_H_

#include "arm_math.h"

void arm_cmplx_mag_fast_q15(
  const q15_t * pSrc,
        q15_t * pDst,
        uint32_t numSamples);

#endif /* INC_ARM_CMPLX_MAG_FAST_Q15_H_ */
