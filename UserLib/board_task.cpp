/*
 * board_task.cpp
 *
 *  Created on: Feb 28, 2024
 *      Author: gomas
 */

#include "board_task.hpp"

namespace G24_STM32HAL::PCUBoard{
	void init(void){
		LED_R.start();
		LED_G.start();
		LED_B.start();

		can.start();
		can.set_filter_free(0);

		HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_val, 2);

		HAL_Delay(10);
		for(int i = 0; i < 16; i++){
			current_sens_offset += adc_val[0];
			HAL_Delay(1);
		}
		current_sens_offset >>= 4;

		buzzer.play(PCULib::SoundData::test);
	}

	void main_data_proccess(void){

	}
	void monitor_task(void){

	}
}


