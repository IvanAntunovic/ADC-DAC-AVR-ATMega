/*
 * AnalogConverter.cpp
 *
 * Created: 5/5/2017 10:10:45 PM
 *  Author: Ivan Antunovi?
 */ 
#include "AnalogConverter.h"

uint16_t volatile	AnalogConverter::sConversionResult = 0;
AnalogConverter*	AnalogConverter::sInstance = NULL;
SemaphoreHandle_t	AnalogConverter::xMutex = NULL;

AnalogConverter* AnalogConverter::getInstance()
{
	if (AnalogConverter::sInstance = NULL)
	{
		AnalogConverter::sInstance = new AnalogConverter();
	}
	return  AnalogConverter::sInstance;
}

AnalogConverter::AnalogConverter(void)
{
	// Make sure we do not do Dynamic Memory Allocation in ISR
	AnalogConverter::getInstance();
	
	//Clear OC1A/OC1B on Compare Match when up counting.
	//Set OC1A/OC1B on Compare Match wheN down counting.
	TCCR1A |= (1 << COM1A0);
	//PWM, Phase Correct, 10-bit
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	//Clock prescaler 64
	TCCR1B |= (1 << CS11) | (1 << CS10);
	OCR1A = 0;
	DDRD |= 1 << PIND5;
	
	ADCSRA |= 1<<ADPS2;
	ADMUX |= (1<<REFS0) | (1<<REFS1);
}

int8_t AnalogConverter::open(void)
{
	if (this->currentMode == eOPENED)
	{
		return PORT_OK;
	}
	this->currentMode = eOPENED;
	
	// Enable ADC Interrupt
	ADCSRA |= 1<<ADIE;
	// Enable ADC module
	ADCSRA |= 1<<ADEN;
	// Enable Global Interrupts
	sei();
	// Start ADC Conversion
	ADCSRA |= 1<<ADSC;
	
	return PORT_NOK;
}

void AnalogConverter::close(void)
{
	ADCSRA &= ~(1<<ADIE);
	ADCSRA &= ~(1<<ADEN);
	this->currentMode = eCLOSED;
}

int16_t AnalogConverter::read(uint8_t pinNumber)
{
	if (this->currentMode == eRUNNING)
	{
		return CONVERSION_NOT_DONE;	
	}
	
	if (pinNumber < 0 || pinNumber > 7)
	{
		return INVALID_PIN_NUMBER;
	}
	
	this->currentMode = eRUNNING;
	if( xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE )
	{
		ADCSRA |= 1 << ADSC;
		this->currentMode = eIDLE;
		return AnalogConverter::sConversionResult;
	}
	
	return CONVERTER_NO_NEW_DATA;
}

void AnalogConverter::write(uint16_t value)
{
	//long mappedValue;
//
	//mappedValue = this->map(value, 0, 1023, 0, 15625);
	//OCR1A = mappedValue;
	OCR1A = value;
}

void AnalogConverter::handleInterrupt()
{
	DDRC ^= 0xFF;
	uint8_t theLowAdcByte;
	
	theLowAdcByte = ADCL;
	AnalogConverter::sConversionResult = ADCH << 8 | theLowAdcByte;
	xSemaphoreGiveFromISR(AnalogConverter::xMutex, false);
}

long AnalogConverter::map(long x, long inMin, long inMax, long outMin, long outMax)
{
	return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}


ISR(ADC_vect)
{
	AnalogConverter::getInstance()->handleInterrupt();
}
