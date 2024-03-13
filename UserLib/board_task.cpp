/*
 * board_task.cpp
 *
 *  Created on: Feb 28, 2024
 *      Author: gomas
 */

#include "board_task.hpp"

namespace G24_STM32HAL::PCUBoard{
	void init(void){
		monitor_timer.set_task(PCUBoard::monitor_task);

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

		can.set_filter_mask(0, 0x00100000|(BOARD_ID<<16), 0x00FF0000, CommonLib::FilterMode::STD_AND_EXT, true);
		can.set_filter_mask(1, 0x00000000|(BOARD_ID<<16), 0x00FF0000, CommonLib::FilterMode::STD_AND_EXT, true);
		can.set_filter_mask(2, 0x00F00000, 0x00F00000, CommonLib::FilterMode::STD_AND_EXT, true);
		can.start();

		LED_R.start();
		LED_G.start();
		LED_B.start();

		led_timer.set_task([](){
			LED_R.update();
			LED_G.update();
			LED_B.update();
		});
		led_timer.set_and_start(1000);
		LED_G.play(PCULib::ok);
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

	void discharge_task(void){
		if(get_emergency_stop_state()){
			HAL_GPIO_WritePin(DISCHARGE_GPIO_Port,DISCHARGE_Pin,GPIO_PIN_RESET);
		}else{
			HAL_GPIO_WritePin(DISCHARGE_GPIO_Port,DISCHARGE_Pin,GPIO_PIN_SET);
		}
	}

	void soft_emergency_stop_task(void){
		if((get_pcu_state() & ems_trigger.get()) || get_soft_emergency_stop()){
			set_soft_emergency_stop(true);
		}else{
			set_soft_emergency_stop(false);
		}
	}

	void emergency_stop_alert_task(void){
		uint8_t pcu_state = get_pcu_state();

		if(pcu_state != old_pcu_state){
			LED_R.play(PCULib::error);
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

			if(common_ems_enable){
				CommonLib::DataPacket common_data;

				common_data.board_ID = BOARD_ID;
				common_data.data_type = CommonLib::DataType::COMMON_DATA_ENFORCE;
				common_data.priority = 0;

				if(get_emergency_stop_state()){
					common_data.register_ID = (uint16_t)PCULib::CommonReg::EMERGENCY_STOP;
				}else{
					common_data.register_ID = (uint16_t)PCULib::CommonReg::RESET_EMERGENCY_STOP;
				}

				CommonLib::CanFrame common_frame;
				CommonLib::DataConvert::encode_can_frame(common_data, common_frame);
				can.tx(common_frame);
			}
		}

		old_pcu_state = pcu_state;
	}

	void communication_task(void){
		if(can.rx_available()){
			CommonLib::DataPacket rx_data;
			CommonLib::CanFrame rx_frame;
			can.rx(rx_frame);
			if(CommonLib::DataConvert::decode_can_frame(rx_frame, rx_data)){
				if(rx_data.board_ID == BOARD_ID && rx_data.data_type == CommonLib::DataType::PCU_DATA){
					execute_pcu_command(BOARD_ID,rx_data);
				}else if((BOARD_ID == rx_data.board_ID && rx_data.data_type == CommonLib::DataType::COMMON_DATA)
						||(rx_data.data_type == CommonLib::DataType::COMMON_DATA_ENFORCE)){
					execute_common_command(BOARD_ID,rx_data);
				}
			}
		}
	}
	void execute_pcu_command(size_t board_id,const CommonLib::DataPacket &rx_data){
		if(rx_data.is_request){
			CommonLib::CanFrame tx_frame;
			CommonLib::DataPacket tx_data;
			auto writer = tx_data.writer();

			if(id_map.get(rx_data.register_ID, writer)){
				tx_data.board_ID = board_id;
				tx_data.data_type = CommonLib::DataType::PCU_DATA;
				tx_data.priority = rx_data.priority;
				tx_data.register_ID = rx_data.register_ID;

				CommonLib::DataConvert::encode_can_frame(tx_data, tx_frame);

				can.tx(tx_frame);
			}
		}else{
			auto reader = rx_data.reader();
			id_map.set(rx_data.register_ID, reader);
		}
	}
	void execute_common_command(size_t board_id,const CommonLib::DataPacket &rx_data){
		CommonLib::DataPacket tx_data;
		CommonLib::CanFrame tx_frame;

		switch((PCULib::CommonReg)rx_data.register_ID){
		case PCULib::CommonReg::NOP:
			break;
		case PCULib::CommonReg::ID_REQEST:
			if(rx_data.is_request){
				tx_data.board_ID = board_id;
				tx_data.data_type = CommonLib::DataType::COMMON_DATA;
				tx_data.register_ID = (uint16_t)PCULib::CommonReg::ID_REQEST;
				tx_data.writer().write<uint8_t>((uint8_t)CommonLib::DataType::PCU_DATA);
				tx_data.priority = rx_data.priority;

				CommonLib::DataConvert::encode_can_frame(tx_data,tx_frame);
				can.tx(tx_frame);
			}
			break;
		case PCULib::CommonReg::EMERGENCY_STOP:
			if(get_emergency_stop_state()){
				//nop
			}else{
				set_soft_emergency_stop(true);
			}
			break;
		case PCULib::CommonReg::RESET_EMERGENCY_STOP:
			set_soft_emergency_stop(false);
			break;
		default:
			break;
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


