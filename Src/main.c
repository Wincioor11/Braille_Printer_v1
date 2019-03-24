
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
static uint8_t table[27]; //max 27 /////liczba wczytywanych znakow - MO¯NA ZMIENIAC!!!
static uint16_t tableLength = 5; ///////////////////////////
static uint16_t fullLineBrailleTable[3][45];
static uint16_t singleChar[3][3];
static uint16_t doubleChar[3][6];
static uint16_t n = 0;
static uint16_t counter = 0;
static uint16_t lineCounter = 0;
static uint16_t markerCounter = 0;

static uint8_t msg[100];

static bool stopFlag = true;
static bool markerFlag = false;
static bool zeroFlag = false;
static bool paperFlag = false;
static bool pwmFlag = false;
static bool resetFlag = false;/////czkeaja do kolejnego etapu prac
static bool loadFlag = false;
static bool isFirstFlag = true;

static uint16_t step = 0;	//////////////rozmiar krokow - MO¯NA ZMIENIAC!!!
static uint16_t x = -1;  //flaga ruchu -1 ¿eby na pocz¹tki ni chuj nic siê nie rusza³o :)
static uint16_t r = -1;
static uint8_t data[100];
static uint16_t size;

static uint16_t pwmCounter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM4_Init(void);                                    
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin == Zero_Sensor_Pin){
	  if(!HAL_GPIO_ReadPin(Zero_Sensor_GPIO_Port, Zero_Sensor_Pin)){

		  zeroFlag = true;
		  //afterResetFunction();
	  }else{

		  zeroFlag = false;

	  }
  }
 if(GPIO_Pin == Paper_Sensor_Pin){
	  if(!HAL_GPIO_ReadPin(Paper_Sensor_GPIO_Port, Paper_Sensor_Pin)){
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, SET);
		  paperFlag = true;
	  }else{
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RESET);
  		  paperFlag = false;

  	  }
  }
}

void numberOfChars(){
	for(int i = 0; i<tableLength; i++){
		if(65 <= table[i] && table[i] <= 90){
			n+=2;
		}else if((48 <= table[i] && table[i] <= 57)){
			if(i == 0 || (i > 0 && (table[i-1] < 48 || table[i-1] > 57))){
				n+=2;
			}else n++;
		}else n++;
	}
}

void stringToBrailleTable(){
	uint16_t localFlag = 0;
	uint16_t wchichChar = 0;
	uint16_t lastChar = 0;

	for(wchichChar = 0; wchichChar<tableLength; wchichChar++){
		// 0 - puste pole 1 - kropka 2 - koniec znaku/przejscie do nast znaku
		//brakuje jeszcze case'ow na znaki zpecjalne itd :)
		switch(table[wchichChar]){
			//ma³e litery! a-z i spacja kropka, przecinek i wykrzyknik
			case 44: singleChar[0][0] = 0;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2;	break;
			case 46: singleChar[0][0] = 0;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2;	break;
			case 33: singleChar[0][0] = 0;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2;	break;
			case 32: singleChar[0][0] = 0;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2;	break;
			case 49:
			case 65:
			case 97: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2;	break;
			case 50:
			case 66:
			case 98: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 51:
			case 67:
			case 99: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 52:
			case 68:
			case 100: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 53:
			case 69:
			case 101: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 54:
			case 70:
			case 102: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 55:
			case 71:
			case 103: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 56:
			case 72:
			case 104: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 57:
			case 73:
			case 105: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 48:
			case 74:
			case 106: singleChar[0][0] = 0;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 75:
			case 107: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 76:
			case 108: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 77:
			case 109: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 78:
			case 110: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case  79:
			case 111: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 80:
			case 112: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 81:
			case 113: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 82:
			case 114: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 83:
			case 115: singleChar[0][0] = 0;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 84:
			case 116: singleChar[0][0] = 0;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 0; singleChar[2][2] = 2; break;
			case 85:
			case 117: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 1; singleChar[2][2] = 2; break;
			case 86:
			case 118: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 1; singleChar[2][2] = 2; break;
			case 87:
			case 119: singleChar[0][0] = 0;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 1;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 0;	singleChar[2][1] = 1; singleChar[2][2] = 2; break;
			case 88:
			case 120: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 0; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 1; singleChar[2][2] = 2; break;
			case 89:
			case 121: singleChar[0][0] = 1;	singleChar[0][1] = 1; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 1; singleChar[2][2] = 2; break;
			case 90:
			case 122: singleChar[0][0] = 1;	singleChar[0][1] = 0; singleChar[0][2] = 2;
					singleChar[1][0] = 0;	singleChar[1][1] = 1; singleChar[1][2] = 2;
					singleChar[2][0] = 1;	singleChar[2][1] = 1; singleChar[2][2] = 2; break;


		}

		if(65 <= table[wchichChar] && table[wchichChar] <= 90){
			//znak poprzedzaj¹cy wielkie litery
			doubleChar[0][0] = 0;	doubleChar[0][1] = 1; doubleChar[0][2] = 2;
			doubleChar[1][0] = 0;	doubleChar[1][1] = 0; doubleChar[1][2] = 2;
			doubleChar[2][0] = 0;	doubleChar[2][1] = 1; doubleChar[2][2] = 2;

			localFlag = 2;


			for(int i = 0; i<3; i++){
				for(int j = 0; j<3; j++){
					doubleChar[i][j+3] = singleChar[i][j];

				}

			}

		}else if(48 <= table[wchichChar] && table[wchichChar] <= 57){
			if(wchichChar == 0 || (wchichChar > 0 && (table[wchichChar-1] < 48 || table[wchichChar-1] > 57))){
				//znak poprzedzaj¹cy liczby//////////////////////////////////////////////
				doubleChar[0][0] = 0;	doubleChar[0][1] = 1; doubleChar[0][2] = 2;
				doubleChar[1][0] = 0;	doubleChar[1][1] = 1; doubleChar[1][2] = 2;
				doubleChar[2][0] = 1;	doubleChar[2][1] = 1; doubleChar[2][2] = 2;

				localFlag = 2;

				for(int i = 0; i<3; i++){
					for(int j = 0; j<3; j++){
						doubleChar[i][j+3] = singleChar[i][j];

					}

				}
			}

		}else localFlag = 1;

		for(int i = 0; i<3; i++){
			for(int j = 0; j<localFlag*3; j++){
				if(localFlag == 1){
					fullLineBrailleTable[i][(wchichChar * 3) + j] = singleChar[i][j];
				}else if (localFlag == 2){
					fullLineBrailleTable[i][(wchichChar * 3) + j] = doubleChar[i][j];
				}

				lastChar = (wchichChar * 3) + j;
			}


		}


	}
	for(int i = 0; i<3; i++){  //uzupe³nienie reszty tablicy spacjami w braille'u
		 	 	 	 	 	 	 /////narazie jest 45 czyli ca³a linia niewazne ile wpiszemy, ale
								//docelowo bêdzie tam n*3 kolumn i nie bedziemy uzupe³niac spacjami :)
		for(int j = lastChar; j<45; j++){
			if((j+1)%3 == 0) fullLineBrailleTable[i][j] = 2;
			else fullLineBrailleTable[i][j] = 0;
		}
	}

}

void reset1(){
	stopFlag = true;
	if(!HAL_GPIO_ReadPin(M1_Dir_GPIO_Port, M1_Dir_Pin))
		HAL_GPIO_WritePin(M1_Dir_GPIO_Port, M1_Dir_Pin, SET);

	TIM1->CCR1 = 50;
	TIM4->CCR3 = 0;
	x = 4;
	pwmFlag = true;
}

void loadPaper(){
	stopFlag = true;
	if(HAL_GPIO_ReadPin(M2_Dir_GPIO_Port, M2_Dir_Pin))
		HAL_GPIO_WritePin(M2_Dir_GPIO_Port, M2_Dir_Pin, RESET);

	TIM1->CCR1 = 0;
	TIM4->CCR3 = 50;
	x = 5;
	pwmFlag = true;
}

void afterMoveFunction(){
	if(x == 0){
		if(fullLineBrailleTable[lineCounter][counter] == 0){
			TIM1->CCR1 = 0;/////////////////////////////////////////////////////////////////
			TIM4->CCR3 = 0;
			x=1;
			step = 0;
			pwmFlag = true;
		}else if(fullLineBrailleTable[lineCounter][counter] == 1){
			x=1;
			markerFlag = true;
		}else if(fullLineBrailleTable[lineCounter][counter] == 2){

			TIM1->CCR1 = 50;/////////////////////////////////////////////////////////////////
			TIM4->CCR3 = 0;
			x=1;
			step = 2;
			pwmFlag = true;
		}

	}else if(x == 1){
		counter++;
		if((counter)%3==0) isFirstFlag = true;

		if(counter >= n*3){
			counter = 0;
			lineCounter++;
			r=1;
			resetFlag=true;/////////////////////////////////////////////////////////////////////
		}else stopFlag = false;
	}else if(x == 2){
		if(lineCounter >= 3){
			lineCounter = 0;
			r=2;
			resetFlag=true;/////////////////////////////////////////////////////////////////////
		}else stopFlag = false;
	}

}

void afterResetFunction(){
	if(r == 0){
		r = 3;
		loadFlag = true;
	}else if(r == 1){
		r = 4;
		loadFlag = true;
	}else if(r == 2){
		r = 5;
		loadFlag = true;
	}else if(r==3){
		n=0;
		numberOfChars();
		stringToBrailleTable();
		///////////////
		size = sprintf(&data, "\n\r Saved: %s , jest to %d znakow "
				"w Braille'u.\n\r Rozpoczynam drukowanie lini...\n\r", table, n);
		//////////////
		HAL_UART_Transmit_IT(&huart2, &data, size);
		stopFlag = false;
	}else if(r == 4){
		//oœ Y ruch
		if(HAL_GPIO_ReadPin(M2_Dir_GPIO_Port, M2_Dir_Pin))
			HAL_GPIO_WritePin(M2_Dir_GPIO_Port, M2_Dir_Pin, RESET);
		TIM1->CCR1 = 0;//////////////////////////////////////////////////////////////////////
		TIM4->CCR3 = 50;
		x = 2;
		step = 35;
		pwmFlag = true;
	}else if(r == 5){
		uint16_t size2 = sprintf(&msg, "Wpisz kolejna linie do druku: \n\r");
		HAL_UART_Transmit_IT(&huart2, &msg, size2);

		HAL_UART_Receive_IT(&huart2, &table, tableLength);
	}

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim2){
		if(!stopFlag){
			stopFlag = true;


			if(HAL_GPIO_ReadPin(M1_Dir_GPIO_Port, M1_Dir_Pin))
					HAL_GPIO_WritePin(M1_Dir_GPIO_Port, M1_Dir_Pin, RESET);
			//zawsze jedn krok i potem program decyduje co dalej
			if(!isFirstFlag){
				TIM1->CCR1 = 50;///////////////////////////////////////////////////////////////////////
				TIM4->CCR3 = 0;
				step = 6;
			}else{
				TIM1->CCR1 = 0;///////////////////////////////////////////////////////////////////////
				TIM4->CCR3 = 0;
				step = 0;
			}
			isFirstFlag = false;
			x = 0;
			pwmFlag = true;

		}
	}
	if(htim == &htim3){
		//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		if(markerFlag){
			markerCounter++;
			HAL_GPIO_TogglePin(Marker_GPIO_Port,  Marker_Pin);
			if(markerCounter== 2){
				markerCounter = 0;
				markerFlag = false;
				afterMoveFunction();
			}

		}if(pwmFlag){
			HAL_GPIO_WritePin(M1_Sleep_GPIO_Port, M1_Sleep_Pin, RESET);
			HAL_GPIO_WritePin(M2_Sleep_GPIO_Port, M2_Sleep_Pin, RESET);
			pwmCounter++;
			if(x == 4){
				if(zeroFlag){
					pwmFlag = false;
					pwmCounter = 0;
					TIM1->CCR1 = 0;
					TIM4->CCR3 = 0;
					HAL_GPIO_WritePin(M1_Sleep_GPIO_Port, M1_Sleep_Pin, SET);
					HAL_GPIO_WritePin(M2_Sleep_GPIO_Port, M2_Sleep_Pin, SET);
					afterResetFunction();
				}
			}else if(x == 5){
				if(paperFlag){
					pwmFlag = false;
					pwmCounter = 0;
					TIM1->CCR1 = 0;
					TIM4->CCR3 = 0;
					HAL_GPIO_WritePin(M1_Sleep_GPIO_Port, M1_Sleep_Pin, SET);
					HAL_GPIO_WritePin(M2_Sleep_GPIO_Port, M2_Sleep_Pin, SET);
					afterResetFunction();
				}
			}
			else if(pwmCounter >= step){//5 to przyk³adowa liczba krokow
				pwmFlag = false;
				pwmCounter = 0;
				TIM1->CCR1 = 0;
				TIM4->CCR3 = 0;
				HAL_GPIO_WritePin(M1_Sleep_GPIO_Port, M1_Sleep_Pin, SET);
				HAL_GPIO_WritePin(M2_Sleep_GPIO_Port, M2_Sleep_Pin, SET);
				afterMoveFunction();
			}
		}

	}
	if(htim == &htim4){
		if(resetFlag){
			resetFlag = false;
			if(!HAL_GPIO_ReadPin(Zero_Sensor_GPIO_Port, Zero_Sensor_Pin)){
				afterResetFunction();
			}else{
				reset1();
			}
		}
		if(loadFlag){
			loadFlag = false;
			  if(!HAL_GPIO_ReadPin(Paper_Sensor_GPIO_Port, Paper_Sensor_Pin)){
				  afterResetFunction();
			  }else{
				  uint16_t size3 = sprintf(&msg, "\n\r UMIESC PAPIER W NAPEDZIE !!! \n\r");
				  HAL_UART_Transmit_IT(&huart2, &msg, size3);
				  loadPaper();
			  }
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	stopFlag = true;
	r=0;
	resetFlag=true;/////////////////////////////////////////////////////////////////////
}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(M1_Sleep_GPIO_Port, M1_Sleep_Pin, SET);
  HAL_GPIO_WritePin(M2_Sleep_GPIO_Port, M2_Sleep_Pin, SET);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  TIM1->CCR1 = 0;
  TIM4->CCR3 = 0;

  uint16_t size1 = sprintf(&msg, "Wpisz linie do druku: \n\r");
  HAL_UART_Transmit_IT(&huart2, &msg, size1);

  HAL_UART_Receive_IT(&huart2, &table, tableLength);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 15999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim1);

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 63999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 499;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 63999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 199;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM4 init function */
static void MX_TIM4_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 15999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 99;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim4);

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|Marker_Pin|M2_Sleep_Pin|M1_Sleep_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(M1_Dir_GPIO_Port, M1_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(M2_Dir_GPIO_Port, M2_Dir_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin Marker_Pin M2_Sleep_Pin M1_Sleep_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|Marker_Pin|M2_Sleep_Pin|M1_Sleep_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Zero_Sensor_Pin Paper_Sensor_Pin */
  GPIO_InitStruct.Pin = Zero_Sensor_Pin|Paper_Sensor_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : M1_Dir_Pin */
  GPIO_InitStruct.Pin = M1_Dir_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(M1_Dir_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : M2_Dir_Pin */
  GPIO_InitStruct.Pin = M2_Dir_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(M2_Dir_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
