/*
 * music_data.hpp
 *
 *  Created on: Feb 26, 2024
 *      Author: yaa3k
 */

#ifndef MUSIC_DATA_HPP_
#define MUSIC_DATA_HPP_

#include "sound.hpp"

namespace G24_STM32HAL::PCULib::SoundData{
	inline const Sound test[]={
			{Scale::C4,1000},
			{Scale::D4,1000},
			{Scale::E4,1000},
			{Scale::F4,1000},
			{Scale::G4,1000},
			{Scale::A4,1000},
			{Scale::B4,1000},
			{Scale::C5,1000},

			{Scale::END,0},
	};

	inline const Sound no_battery[] = {{Scale::C3,200},{Scale::C4,200},{Scale::C5,200},{Scale::END,0}};
	inline const Sound cell1[] = {{Scale::C4,200},{Scale::END,0},};
	inline const Sound cell2[] = {{Scale::C4,200},{Scale::D4,200},{Scale::END,0},};
	inline const Sound cell3[] = {{Scale::C4,200},{Scale::D4,200},{Scale::E4,200},{Scale::END,0},};
	inline const Sound cell4[] = {{Scale::C4,200},{Scale::D4,200},{Scale::E4,200},{Scale::F4,200},{Scale::END,0},};
	inline const Sound cell5[] = {{Scale::C4,200},{Scale::D4,200},{Scale::E4,200},{Scale::F4,200},{Scale::G4,200},{Scale::END,0},};
	inline const Sound cell6[] = {{Scale::C4,200},{Scale::D4,200},{Scale::E4,200},{Scale::F4,200},{Scale::G4,200},{Scale::A4,200},{Scale::END,0},};

	inline const auto start_theme = std::array<const Sound*,7>{
			no_battery,cell1,cell2,cell3,cell4,cell5,cell6
	};

	inline const Sound emergency_stop[]={
			{Scale::C5,500},
			{Scale::OFF,100},
			{Scale::C5,500},

			{Scale::END,0},
	};
	inline const Sound soft_emergency_stop[]={
			{Scale::C5,500},
			{Scale::OFF,100},
			{Scale::C5,100},
			{Scale::OFF,100},
			{Scale::C5,500},

			{Scale::END,0},
	};
	inline const Sound safe[]={
			{Scale::C5,100},
			{Scale::OFF,100},
			{Scale::C5,100},

			{Scale::END,0},
	};
	inline const Sound over_voltage_theme[]={
			{Scale::C5,100},
			{Scale::D5,100},
			{Scale::G5,100},
			{Scale::OFF,100},
			{Scale::C5,100},
			{Scale::D5,100},
			{Scale::G5,100},
			{Scale::END,0},
	};
	inline const Sound under_voltage_theme[]={
			{Scale::G5,100},
			{Scale::D5,100},
			{Scale::C5,100},
			{Scale::OFF,100},
			{Scale::G5,100},
			{Scale::D5,100},
			{Scale::C5,100},
			{Scale::END,0},
	};
	inline const Sound over_current_theme[]={
			{Scale::G5,100},
			{Scale::D5,100},
			{Scale::C5,100},
			{Scale::OFF,100},
			{Scale::C5,100},
			{Scale::D5,100},
			{Scale::G5,100},
			{Scale::END,0},
	};

}

#endif /* MUSIC_DATA_HPP_ */
