/*
 * board_task.hpp
 *
 *  Created on: Feb 25, 2024
 *      Author: yaa3k
 */

#ifndef BOARD_TASK_HPP_
#define BOARD_TASK_HPP_


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

	inline auto LED_R = CommonLib::PWMHard{&htim3,TIM_CHANNEL_4};
	inline auto LED_G = CommonLib::PWMHard{&htim3,TIM_CHANNEL_2};
	inline auto LED_B = CommonLib::PWMHard{&htim3,TIM_CHANNEL_1};

	inline auto buzzer = CommonLib::PWMHard{&htim2,TIM_CHANNEL_2};

	inline auto can = CommonLib::CanComm<4,4>(&hcan,CAN_RX_FIFO0,CAN_FILTER_FIFO0,CAN_IT_RX_FIFO0_MSG_PENDING);

	inline uint16_t adc_val[2] = {0};

	void init(void){
		LED_R.start();
		LED_G.start();
		LED_B.start();

		buzzer.start();

		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_val, 2);
	}
}

#endif /* BOARD_TASK_HPP_ */
