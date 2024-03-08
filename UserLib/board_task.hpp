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
#include "board_id.hpp"

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
#include <cmath>

namespace G24_STM32HAL::PCUBoard{

	inline constexpr uint8_t BOARD_ID = 0x0;

	//peripherals
	//TIMER/PWM
	inline auto *sound_control_timer = &htim16;
	inline auto *monitor_timer = &htim17;

	inline auto LED_R = CommonLib::PWMHard{&htim3,TIM_CHANNEL_4};
	inline auto LED_G = CommonLib::PWMHard{&htim3,TIM_CHANNEL_2};
	inline auto LED_B = CommonLib::PWMHard{&htim3,TIM_CHANNEL_1};

	inline auto buzzer = PCULib::Buzzer{sound_control_timer,&htim2,TIM_CHANNEL_3};

	//CAN
	inline auto can = CommonLib::CanComm<4,4>(&hcan,CAN_RX_FIFO0,CAN_FILTER_FIFO0,CAN_IT_RX_FIFO0_MSG_PENDING);

	//ADC
	inline uint16_t adc_val[2] = {0};
	inline uint16_t current_sens_offset = 0;
	inline auto get_voltage = []()->float{ return (float)adc_val[1]*11.0f*3.3f/4096.0f; };
	inline auto get_current = []()->float{ return (float)(adc_val[0]-current_sens_offset)*3.3f/(0.015f*4096.0f); };

	//battery management
	inline uint8_t cell_n = 6;
	inline float voltage_limit_high = 4.3f;
	inline float voltage_limit_low = 3.7f;

	inline auto is_over_voltage = []()->bool{
		if(cell_n==0){return false;}
		return voltage_limit_high*cell_n < get_voltage() ? true : false;
	};
	inline auto is_under_voltage = []()->bool{
		if(cell_n==0){return false;}
		return voltage_limit_low*cell_n > get_voltage() ? true : false;
	};

	//current management
	inline float current_limit = 100.0f;
	inline auto is_over_current = []()->bool{return current_limit < std::abs(get_current()) ? true:false;};

	//emergency stop management
	struct EmergencyStopTrigger{
		bool over_voltage_stop = false;
		bool under_voltage_stop = false;
		bool over_current_stop = false;
		void set(uint8_t val){
			over_voltage_stop = (val >> 2)&1u;
			under_voltage_stop = (val >> 3)&1u;
			over_current_stop = (val >> 4)&1u;
		}
		uint8_t get(void){
			return (over_voltage_stop?1<<2:0)
					|(under_voltage_stop?1<<3:0)
					|(over_current_stop?1<<4:0);
		}
	};

	inline auto ems_trigger = EmergencyStopTrigger{};
	inline auto set_soft_emergency_stop = [](bool state) { HAL_GPIO_WritePin(POWER_SD_GPIO_Port,POWER_SD_Pin,state?GPIO_PIN_SET:GPIO_PIN_RESET); };

	inline auto get_soft_emergency_stop = []()->bool { return HAL_GPIO_ReadPin(POWER_SD_GPIO_Port,POWER_SD_Pin)?true:false;};
	inline auto get_emergency_stop_state = []()->bool{ return HAL_GPIO_ReadPin(EM_CHECK_GPIO_Port,EM_CHECK_Pin); };

	inline auto get_pcu_state = []()->uint8_t{
		return (get_emergency_stop_state()?1:0)
				|(get_soft_emergency_stop()?1<<1:0)
				|(is_over_voltage()?1<<2:0)
				|(is_under_voltage()?1<<3:0)
				|(is_over_current()?1<<4:0);
	};
	inline uint8_t old_pcu_state = 0;

	//timer control
	inline auto set_monitor_period = [](uint16_t val){
		if(val == 0){
			HAL_TIM_Base_Stop_IT(monitor_timer);
		}else{
			__HAL_TIM_SET_AUTORELOAD(monitor_timer,val);
			__HAL_TIM_SET_COUNTER(monitor_timer,0);

			if(HAL_TIM_Base_GetState(monitor_timer) == HAL_TIM_STATE_READY){
				HAL_TIM_Base_Start_IT(monitor_timer);
			}
		}
	};
	inline auto get_monitor_period = []()->uint16_t{
		if(HAL_TIM_Base_GetState(monitor_timer) == HAL_TIM_STATE_BUSY)return __HAL_TIM_GET_AUTORELOAD(monitor_timer);
		else return 0;
	};

	//monitor
	inline auto monitor = std::bitset<0x23>{};

	inline auto id_map = CommonLib::IDMapBuilder()
			.add((uint16_t)PCULib::PCUReg::PCU_STATE,     CommonLib::DataAccessor::generate<uint8_t>(get_pcu_state))
			.add((uint16_t)PCULib::PCUReg::EX_EMS_TRG,    CommonLib::DataAccessor::generate<uint8_t>([](uint8_t v) {ems_trigger.set(v);},[]()->uint8_t{return ems_trigger.get();}))
			.add((uint16_t)PCULib::PCUReg::CELL_N,        CommonLib::DataAccessor::generate<uint8_t>(cell_n))
			.add((uint16_t)PCULib::PCUReg::EMS_RQ,        CommonLib::DataAccessor::generate<bool>(set_soft_emergency_stop,get_soft_emergency_stop))
			.add((uint16_t)PCULib::PCUReg::OUT_V,         CommonLib::DataAccessor::generate<float>(get_voltage))
			.add((uint16_t)PCULib::PCUReg::V_LIMIT_HIGH,  CommonLib::DataAccessor::generate<float>(voltage_limit_high))
			.add((uint16_t)PCULib::PCUReg::V_LIMIT_LOW,   CommonLib::DataAccessor::generate<float>(voltage_limit_low))
			.add((uint16_t)PCULib::PCUReg::OUT_I,         CommonLib::DataAccessor::generate<float>(get_current))
			.add((uint16_t)PCULib::PCUReg::I_LIMIT,       CommonLib::DataAccessor::generate<float>(current_limit))
			.add((uint16_t)PCULib::PCUReg::MONITOR_PERIOD,CommonLib::DataAccessor::generate<float>(set_monitor_period,get_monitor_period))
			.add((uint16_t)PCULib::PCUReg::MONITOR_REG,   CommonLib::DataAccessor::generate<uint64_t>([](uint64_t val){ monitor = std::bitset<0x23>{val};}, []()->uint64_t{ return monitor.to_ullong();}))
			.build();

	void init(void);

	uint8_t estimate_battery_cell(void);

	void discharge_task(void);

	void soft_emergency_stop_task(void);

	void emergency_stop_alert_task(void);

	void communication_task(void);
	void execute_pcu_command(size_t board_id,const CommonLib::DataPacket &rx_data);
	void execute_common_command(size_t board_id,const CommonLib::DataPacket &rx_data);

	void monitor_task(void);

}

#endif /* BOARD_TASK_HPP_ */
