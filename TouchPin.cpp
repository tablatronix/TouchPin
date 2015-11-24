#include "Arduino.h"
#include "TouchPin.h"

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
	unsigned int count = 0;

	pinMode( _pin, OUTPUT );
	digitalWrite( _pin, 1 );

	pinMode( _pin, INPUT );

	while( digitalRead( _pin ) ){

		count++;
	}

	return count;
}

long TouchPin::_touchTime() {

	unsigned long now = millis(),
	              result = 0
				  ;;
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

int TouchPin::read()
{
	int result=0, i;
	
	for( i=0; i<MEASURE_RUNS; i++ ) {
		result += _read() - _offset;
	}
	result /= MEASURE_RUNS;

	if( result < 0 ) result = 0;

	return result;
}

int TouchPin::calibrate()
{
	int i;
	int sum = 0;

	for( i = 0; i < CALIBRATION_RUNS; i++ ) {

		sum += _read();
	}

	_offset = sum / CALIBRATION_RUNS;
	_hysteresis = _offset / 2;

	return _offset;
}
