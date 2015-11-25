#include "Arduino.h"
#include "TouchPin.h"
#include "InterruptGuard.h"

#define SCALE( offset ) (offset)
#define HYSTERESIS( offset ) ((offset)/2)

const int CALIBRATION_RUNS = 16,
          MEASURE_RUNS = 4
		  ;

const int MIN_CLICK = 50,
          MIN_HOLD = 200,
		  MIN_PUSH = 50; //ms


TouchPin::TouchPin( int pin )
{
	_pin = pin;
	_offset = 0;
	_touchStart = 0;
	_hysteresis = 0;
	_pushed = false;

	pinMode( _pin, INPUT );
}

int TouchPin::_read()
{
	register int count = 0;

	{
		InterruptGuard g();

		pinMode( _pin, OUTPUT );
		digitalWrite( _pin, 1 );

		pinMode( _pin, INPUT );

		while( digitalRead( _pin ) ){

			count++;
		}

	} 

	return count;
}

int TouchPin::calibrate()
{
	register int i;

	int sum = 0,
		offset;

	for( i = 0; i < CALIBRATION_RUNS; i++ ) {

		sum += _read();
	}

	offset = sum / CALIBRATION_RUNS;
	_offset = offset;
	_hysteresis = HYSTERESIS( offset );
	_scale = SCALE( offset );

	return _offset;
}

int TouchPin::read()
{
	register int i;
	int result=0;
	
	for( i=0; i<MEASURE_RUNS; i++ ) {

		result += _read() - _offset;
	}
	result /= MEASURE_RUNS;

	if( result < 0 ) result = 0;

	if( result ) _lastCount = result;

	return result;
}

long TouchPin::_touchTime()
{
	unsigned long now = millis(),
	              result = 0
				  ;
	bool touching;

	if( _touchStart ) {
		result = now - _touchStart;
	}

	touching = isTouch();

	if( touching && !_touchStart ) _touchStart = now;
	else if( !touching ) _touchStart = 0;

	return touching ? result : -result;
}

bool TouchPin::isTouch()
{
	return read() >= _hysteresis;;
}

bool TouchPin::isClick()
{
	return _touchTime() < -MIN_CLICK;
}

bool TouchPin::isHold()
{
	return _touchTime() > MIN_HOLD;
}

bool TouchPin::isPush()
{
	if( _touchTime() > MIN_PUSH ) {
		if( ! _pushed ){
			_pushed = true;
			return true;
		}
	} else {
		_pushed = false;
	}
	return false;
}

int TouchPin::strength()
{
	return _lastCount / _scale;
}
