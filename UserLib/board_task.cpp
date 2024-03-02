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

		HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_val, 2);

		HAL_Delay(10);
		for(int i = 0; i < 16; i++){
			current_sens_offset += adc_val[0];
			HAL_Delay(1);
		}
		current_sens_offset >>= 4;

		cell_n = estimate_battery_cell();
		buzzer.play(PCULib::SoundData::start_theme.at(cell_n));
		set_soft_emergency_stop(false);
		while(buzzer.is_playing());

		old_pcu_state = get_pcu_state();

		can.start();
		can.set_filter_free(0);
	}

	uint8_t estimate_battery_cell(void){
		float voltage = get_voltage();
		if(voltage<voltage_limit_low){
			return 0;
		}
		for(int i = 1; i <= 6; i++){
			if(voltage_limit_low*i<voltage && voltage<voltage_limit_high*i){
				return i;
			}
		}
		return 0;
	}

	void soft_emergency_stop_task(void){
		if(get_pcu_state() & ems_trigger.get()){
			set_soft_emergency_stop(true);
		}else{
			set_soft_emergency_stop(false);
		}
	}

	void emergency_stop_alert_task(void){
		uint8_t pcu_state = get_pcu_state();

		if(pcu_state != old_pcu_state){
			CommonLib::DataPacket tx_data;
			tx_data.board_ID = BOARD_ID;
			tx_data.data_type = CommonLib::DataType::PCU_DATA;
			tx_data.register_ID = (uint16_t)PCULib::PCUReg::PCU_STATE;
			tx_data.priority = 0;

			tx_data.writer().write(pcu_state);

			CommonLib::CanFrame tx_frame;
			CommonLib::DataConvert::encode_can_frame(tx_data, tx_frame);
			can.tx(tx_frame);

			if(is_over_voltage()){
				buzzer.play(PCULib::SoundData::over_voltage_theme);
			}else if(is_under_voltage()){
				buzzer.play(PCULib::SoundData::under_voltage_theme);
			}else if(is_over_current()){
				buzzer.play(PCULib::SoundData::over_current_theme);
			}else if(get_soft_emergency_stop()){
				buzzer.play(PCULib::SoundData::soft_emergency_stop);
			}else if(get_emergency_stop_state()){
				buzzer.play(PCULib::SoundData::emergency_stop);
			}else if(pcu_state==0){
				buzzer.play(PCULib::SoundData::safe);
			}
		}

		old_pcu_state = pcu_state;
	}

	void communication_task(void){
		if(can.rx_available()){
			CommonLib::DataPacket rx_data;
			CommonLib::CanFrame rx_frame;
			can.rx(rx_frame);
			CommonLib::DataConvert::decode_can_frame(rx_frame, rx_data);

			if(rx_data.board_ID == BOARD_ID && rx_data.data_type == CommonLib::DataType::PCU_DATA){
				if(rx_data.is_request){
					CommonLib::CanFrame tx_frame;
					auto writer = tx_frame.writer();

					if(id_map.get(rx_data.register_ID, writer)){
						tx_frame.id = rx_frame.id;
						tx_frame.is_ext_id = true;
						tx_frame.is_remote = false;

						can.tx(tx_frame);
					}
				}else{
					auto reader = rx_data.reader();
					id_map.set(rx_data.register_ID, reader);
				}
			}
		}
	}
	void monitor_task(void){
		for(auto &map_element : id_map.accessors_map){
			if(map_element.first < monitor.size()){
				if(monitor.test(map_element.first)){
					CommonLib::DataPacket tx_packet;
					CommonLib::CanFrame tx_frame;
					tx_packet.register_ID = map_element.first;
					tx_packet.board_ID = BOARD_ID;
					tx_packet.data_type = CommonLib::DataType::PCU_DATA;

					auto writer = tx_packet.writer();
					if(map_element.second.get(writer)){
						CommonLib::DataConvert::encode_can_frame(tx_packet, tx_frame);
						can.tx(tx_frame);
					}
				}
			}
		}
	}
}


