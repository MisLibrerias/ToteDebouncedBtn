#ifndef TOTE_DEBOUNCED_BTN_H
#define TOTE_DEBOUNCED_BTN_H

//#define _TOTE_DEBOUNCED_BTN_DEBUG_ON_ // Descomentar para mostrar info de debug en Serial.

// Macros para facilitar la salida de información por el Serial.
#ifdef _TOTE_DEBOUNCED_BTN_DEBUG_ON_
#define _TOTE_DEBOUNCED_BTN_DEBUG_(type, text) Serial.print("("); Serial.print(millis()); Serial.print(" millis)"); Serial.print(" ["); Serial.print(type); Serial.print("] "); Serial.println(text);
#define _TOTE_DEBOUNCED_BTN_DEBUG_VALUE_(type, text, value) Serial.print("("); Serial.print(millis()); Serial.print(" millis)"); Serial.print(" ["); Serial.print(type); Serial.print("] "); Serial.print(text); Serial.println(value);
#else
#define _TOTE_DEBOUNCED_BTN_DEBUG_(type, text) void();
#define _TOTE_DEBOUNCED_BTN_DEBUG_VALUE_(type, text, value) void();
#endif

#include <Arduino.h>

#ifndef TOTE_ASYNC_DELAY_H
#include "C:\Users\Antonio\OneDrive\MIS COSAS\Proyectos Electronica\Arduino\Mis_Librerias\ToteAsyncDelay\ToteAsyncDelay.cpp"
#endif




/* 
 *  CLASE ToteDeBouncedBtn
 *  
 *  Esta clase realiza la lectura de un switch o pulsador en el dispositivo eliminando el rebote.
 *
 *  Es conocido que cuando se pulsa un botón en el dispositivo se producen lecturas erráticas durante un periodo pequeño de tiempo. Esto es debido a que
 *  conforme se cierra el interruptor, se producen transiciones entre circuito cerrado y abierto, debido a las imperfecciones de los materiales que forman
 *  los contactos. A este fenómeno se le llama 'rebote' (bounce)
 *
 *  Es posible limitarlo poniendo un condensador entre los bornes que forman el interruptor, pero el mejor resultado lo he conseguido realizando una pausa
 *  de pocos milisegundos (15 ms es suficiente) para dar tiempo a que se estabilice la conductividad en el interruptor.
 *
 *  Esta clase hereda de 'ToteAsyncDelay' y permite leer los pulsadores e interruptores de forma efectiva.
 *  
 *  Uso del constructor: Instanciar el objeto de la forma siguiente:
 *
 *        ToteDebouncedBtn  myDebouncedBtn = ToteDebouncedBtn(1, NULL);			// Enlazada al pin 1 de arduino y no se llama a función de callback.
 *        ToteDebouncedBtn  myDebouncedBtn = ToteDebouncedBtn(1, &myCallback);	// Enlazada al pin 1 de arduino y se llama al finalizar el intervalo.
 *        
 * Métodos:
 *
 *		  myDebouncedBtn.init();     // Configura el pin de Arduino como entrada. Este método debe ser llamado desde 'setup()'	
 *        myDebouncedBtn.check();    // (HEREDADO) Debe llamarse en el cuerpo de 'loop()' para leer el estado del pin de arduino
 *        myDebouncedBtn.getState(); // Se puede llamar en cualquier parte del sketch y sirve para leer el estado del pulsador segun la enumeración 'BTN_STATE' (UNDEFINED, DOWN, PUSHED)
 */

class ToteDebouncedBtn : public ToteAsyncDelay {
	public:
		enum BTN_STATE {UNDEFINED, DOWN, PUSHED};	// Posible estado del botón.
		
		ToteDebouncedBtn(uint8_t thePin, void (*theCallback)());
		void init(void);
		void init(uint8_t thePinMode);
	
		BTN_STATE getState(void);
		boolean check(void);

	private:
		const unsigned long BOUNCE_DELAY = 15UL;
		uint8_t _pin;			// Almacena el pin de arduino asociado con el switch
		BTN_STATE _buttonState;	// Indica estado de pulsación del botón.
		uint8_t _pinValue;		// Almacena el estado del pin (HIGH o LOW)
};


// Constructor
ToteDebouncedBtn::ToteDebouncedBtn(uint8_t thePin, void (*theCallback)()) : ToteAsyncDelay(BOUNCE_DELAY, theCallback) {
	_pin = thePin;				// Pin de Arduino.
	_buttonState = UNDEFINED;	// Estado actual de botón.
}

// Inicializo el objeto poniendo el pin de Arduino como entrada.
void ToteDebouncedBtn::init(void) {
	pinMode(_pin, INPUT);
}

// Inicializo el objeto poniendo el pin de Arduino como entrada.
void ToteDebouncedBtn::init(uint8_t thePinMode) {
	pinMode(_pin, thePinMode);
}

// Devuelve el estado del botón.
ToteDebouncedBtn::BTN_STATE ToteDebouncedBtn::getState(void) {
	return _buttonState;
}

// Comprueba si el botón cambia de estado para determinar si se ha producido una pulsación (pulsar y soltar)
boolean ToteDebouncedBtn::check(void) {
	// Leo el pin de arduino.
	_pinValue = digitalRead(_pin);
		
	// Detección de condición inicial. 'HIGH' en el pin y estado de botón en 'UNDEFINED'
	if (_pinValue == HIGH && _buttonState == UNDEFINED) {
		resetCounter();		  // empiezo a contar el intervalo desde cero.
		_buttonState = DOWN;  // Actualizo el estado de pulsación del botón.	
		
		_TOTE_DEBOUNCED_BTN_DEBUG_VALUE_("DEBOUNCE", "Detectado estado HIGH en pin ", _pin);
		_TOTE_DEBOUNCED_BTN_DEBUG_("DEBOUNCE", "Reseteando valor de contador interno. ");
	
		return false; // Nada más que hacer por ahora.
	}
	
	// Si se cumple el intervalo de debouncing y el botón tiene su estado DOWN y sigo leyendo HIGH en el pin, entonces actualizo estado a PUSHED
	if (ToteAsyncDelay::check() && _buttonState == DOWN && (digitalRead(_pin) == HIGH)) {	
	
		_TOTE_DEBOUNCED_BTN_DEBUG_VALUE_("DEBOUCE", "Se ha cumplido el intervalo en ms de ", getInterval());
		_TOTE_DEBOUNCED_BTN_DEBUG_("DEBOUCE", "y el botón sigue pulsado. Actualizo su estado a PUSHED.");
			
		_buttonState = PUSHED; // Estado del botón pulsado. Ahora hay que esperar a que se libere.
		resetCounter(); // Vuelvo a iniciar la cuenta del temporizador.
		
		return false; 
	}
		
	// Si se cumple el intervalo de debouncing y el botón tiene su estado PUSHED y sigo leyendo LOW en el pin, 
	// entonces actualizo estado a UNDEFINED e indico que se ha produción la pulsación.
	if (ToteAsyncDelay::check() && _buttonState == PUSHED && (digitalRead(_pin) == LOW)) {
			
		_TOTE_DEBOUNCED_BTN_DEBUG_VALUE_("DEBOUCE", "Se ha cumplido el intervalo en ms de ", getInterval());
		_TOTE_DEBOUNCED_BTN_DEBUG_("DEBOUCE", "y el botón NO sigue pulsado. Actualizo su estado a UNDEFINED e indico que se ha producido el clic.");
			
		_buttonState = UNDEFINED; // Estado del botón pulsado. Ahora hay que esperar a que se libere.
		resetCounter(); // Vuelvo a iniciar la cuenta del temporizador.
		
		return true; // Se dan las condiciones para entender que se ha producido una pulsación.
	}
	
	return false; // Aún no se puede entender que se ha producido la pulsación del botón.
}
#endif