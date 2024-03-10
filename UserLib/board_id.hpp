/*
 * board_id.hpp
 *
 *  Created on: Feb 28, 2024
 *      Author: gomas
 */

#ifndef BOARD_ID_HPP_
#define BOARD_ID_HPP_

#include <stdint.h>

namespace G24_STM32HAL::PCULib{
	enum class CommonReg:uint16_t{
		NOP,
		ID_REQEST,
		EMERGENCY_STOP = 0xE,
		RESET_EMERGENCY_STOP = 0xF,
	};
	enum class PCUReg : uint16_t{
		NOP,
		PCU_STATE,
		CELL_N,
		EX_EMS_TRG,
		EMS_RQ,
		COMMON_EMS_EN,

		OUT_V = 0x10,
		V_LIMIT_HIGH,
		V_LIMIT_LOW,

		OUT_I = 0x20,
		I_LIMIT,

		MONITOR_PERIOD = 0xF0,
		MONITOR_REG,
	};
}



#endif /* BOARD_ID_HPP_ */
