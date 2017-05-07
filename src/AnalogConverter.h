/*
 * AnalogConverter.cpp
 *
 * Created: 5/5/2017 10:11:32 PM
 *  Author: Ivan Antunovi?
 */

#ifndef  _ANALOG_CONVERTER_H_
#define  _ANALOG_CONVERTER_H_
 
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#ifndef  NULL
#define  NULL 0
#endif

#define CONVERTER_NO_NEW_DATA	-1
#define INVALID_PIN_NUMBER		-2
#define CONVERSION_NOT_DONE		-3

#define PORT_OK		 0
#define PORT_NOK	-1

class AnalogConverter
{
	private:
		static volatile uint16_t sConversionResult;
		static AnalogConverter* sInstance;
		static SemaphoreHandle_t xMutex;
		int currentMode;
		enum _t_AnalogConverterType
		{
			eOPENED,
			eCLOSED,
			eRUNNING,
			eIDLE
		};
	
	public:	
		long map(long x, long inMin, long inMax, long outMin, long outMax);

	public:
		AnalogConverter(void);
		static AnalogConverter* getInstance();
		int8_t open(void);
		void close(void);
		int16_t read(uint8_t pinNumber);
		void write(uint16_t value);
		static inline void handleInterrupt(void);
};

#endif