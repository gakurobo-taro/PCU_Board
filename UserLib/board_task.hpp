/*
 * board_task.hpp
 *
 *  Created on: Feb 25, 2024
 *      Author: yaa3k
 */

#ifndef BOARD_TASK_HPP_
#define BOARD_TASK_HPP_

#include "sound.hpp"
#include "music_data.hpp"

#include "STM32HAL_CommonLib/can_comm.hpp"
#include "STM32HAL_CommonLib/pwm.hpp"
#include "STM32HAL_CommonLib/data_packet.hpp"
#include "STM32HAL_CommonLib/data_convert.hpp"
#include "STM32HAL_CommonLib/serial_comm.hpp"
#include "STM32HAL_CommonLib/id_map_control.hpp"

#include "main.h"
#include "can.h"
#include "tim.h"
#include "gpio.h"
#include "adc.h"

#include <array>
#include <bitset>

namespace G24_STM32HAL::PCUBoard{

	inline auto *sound_control_timer = &htim16;
	inline auto *monitor_timer = &htim17;

	inline auto LED_R = CommonLib::PWMHard{&htim3,TIM_CHANNEL_4};
	inline auto LED_G = CommonLib::PWMHard{&htim3,TIM_CHANNEL_2};
	inline auto LED_B = CommonLib::PWMHard{&htim3,TIM_CHANNEL_1};

	inline auto buzzer = PCULib::Buzzer{sound_control_timer,&htim2,TIM_CHANNEL_3};

	inline auto can = CommonLib::CanComm<4,4>(&hcan,CAN_RX_FIFO0,CAN_FILTER_FIFO0,CAN_IT_RX_FIFO0_MSG_PENDING);

	//ADC
	inline uint16_t adc_val[2] = {0};
	inline uint16_t current_sens_offset = 0;
	auto get_voltage = []()->float{ return (float)adc_val[1]*11.0f*3.3f/4096.0f; };
	auto get_current = []()->float{ return (float)(adc_val[0]-current_sens_offset)*3.3f/(0.015f*4096.0f); };

	auto set_EMS = [](bool state) { return HAL_GPIO_WritePin(POWER_SD_GPIO_Port,POWER_SD_Pin,state?GPIO_PIN_SET:GPIO_PIN_RESET); };
	auto get_EMS_state = []()->bool{ return HAL_GPIO_ReadPin(EM_CHECK_GPIO_Port,EM_CHECK_Pin); };

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

	bool get_EMS_SW_state(void);

	void monitor_task(void){

	}

}

#endif /* BOARD_TASK_HPP_ */
