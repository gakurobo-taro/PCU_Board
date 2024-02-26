/*
 * sound.hpp
 *
 *  Created on: 2024/02/26
 *      Author: yaa3k
 */

#ifndef SOUND_HPP_
#define SOUND_HPP_

#include "tim.h"
#include <stdio.h>

namespace G24_STM32HAL::PCULib{

	//36Mhz timer
    enum class Scale:uint32_t{
    	OFF = 0,
    	B2 = 291567,
    	C3 = 275202,
    	C3_ = 259757,
    	D3 = 245178,
    	D3_ = 231417,
    	E3 = 218428,
    	F3 = 206169,
    	F3_ = 194598,
    	G3 = 183676,
    	G3_ = 173367,
    	A3 = 163636,
    	A3_ = 154452,
    	B3 = 145783,
    	C4 = 137601,
    	C4_ = 129878,
    	D4 = 122589,
    	D4_ = 115708,
    	E4 = 109214,
    	F4 = 103084,
    	F4_ = 97299,
    	G4 = 91838,
    	G4_ = 86683,
    	A4 = 81818,
    	A4_ = 77226,
    	B4 = 72892,
    	C5 = 68801,
    	C5_ = 64939,
    	D5 = 61294,
    	D5_ = 57854,
    	E5 = 54607,
    	F5 = 51542,
    	F5_ = 48649,
    	G5 = 45919,
    	G5_ = 43342,
    	A5 = 40909,
    	A5_ = 38613,
    	B5 = 36446,
    };


    struct Sound{
    	uint32_t sound = 0;
    	uint32_t length = 0; //ms
    	Sound(uint32_t _sound,uint32_t _length):sound(_sound),length(_length){}
    	Sound(Scale _sound,uint32_t _length):sound((uint32_t)_sound),length(_length){}
    };

	class Buzzer{
	private:
    	TIM_HandleTypeDef *length_tim;
		TIM_HandleTypeDef *buzzer_tim;
		const uint32_t ch;

		Sound *playing_music = nullptr;
		int music_length = 0;
		int music_count = 0;

	public:
		Buzzer(TIM_HandleTypeDef *_length_tim,TIM_HandleTypeDef *_buzzer_tim, uint32_t _ch)
			:length_tim(_length_tim),buzzer_tim(_buzzer_tim),ch(_ch){}

		void tone(int period){
			if(period == 0){
				HAL_TIM_PWM_Stop(buzzer_tim, ch);
			}else{
				__HAL_TIM_SET_AUTORELOAD(buzzer_tim,period);
				__HAL_TIM_SET_COUNTER(buzzer_tim,0);
				__HAL_TIM_SET_COMPARE(buzzer_tim, ch,period/2);

				if(HAL_TIM_Base_GetState(buzzer_tim) == HAL_TIM_STATE_READY){
					HAL_TIM_PWM_Start(buzzer_tim, ch);
				}
			}
		}

		void play(Sound *music, size_t s_n){
			playing_music = music;
			music_count = 0;
			music_length = s_n;

			__HAL_TIM_SET_AUTORELOAD(length_tim,playing_music[music_count].length);
			__HAL_TIM_SET_COUNTER(length_tim,0);
			HAL_TIM_Base_Start_IT(length_tim);
		}

		void timer_interrupt_task(void){
			if(playing_music != nullptr){
				tone(playing_music[music_count].sound);

				if(music_count > music_length){
					HAL_TIM_Base_Stop_IT(length_tim);
					playing_music = nullptr;
					return;
				}

				tone((int)playing_music[music_count].sound);

				__HAL_TIM_SET_AUTORELOAD(length_tim,playing_music[music_count].length);
				__HAL_TIM_SET_COUNTER(length_tim,0);

				music_count ++;

				if(music_count >= music_length){
					playing_music = nullptr;
				}
			}else{
				HAL_TIM_Base_Stop_IT(length_tim);
				tone((int)Scale::OFF);
			}
		}
	};



}


#endif /* SOUND_HPP_ */
