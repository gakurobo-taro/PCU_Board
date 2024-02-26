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
	Sound test_music[]={
			{Scale::C4,1000},
			{Scale::OFF,1000},
			{Scale::D4,1000},
			{Scale::OFF,1000},
			{Scale::E4,1000},
			{Scale::OFF,1000},
			{Scale::F4,1000},
			{Scale::OFF,1000},
			{Scale::G4,1000},
			{Scale::OFF,1000},
			{Scale::A4,1000},
			{Scale::OFF,1000},
			{Scale::B4,1000},
			{Scale::OFF,1000},
			{Scale::C5,1000},
	};

	Sound SOS[]={
			{Scale::A4,100},
			{Scale::OFF,100},
			{Scale::A4,100},
			{Scale::OFF,100},
			{Scale::A4,100},
			{Scale::OFF,100*3},

			{Scale::A4,100*3},
			{Scale::OFF,100},
			{Scale::A4,100*3},
			{Scale::OFF,100},
			{Scale::A4,100*3},
			{Scale::OFF,100*3},

			{Scale::A4,100},
			{Scale::OFF,100},
			{Scale::A4,100},
			{Scale::OFF,100},
			{Scale::A4,100},
			{Scale::OFF,100*3},
	};

}

#endif /* MUSIC_DATA_HPP_ */
