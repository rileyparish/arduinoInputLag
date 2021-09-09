#include <Keyboard.h>
#include <movingAvg.h>
#include "ListLib.h"

// “Sketch” menu > Include Library > Manage Libraries… > Search for “movingavg” > Install
// the Arduino must be an HID type; Micro, Leonardo, or Due. The computer doesn't recognize the Uno as a keyboard.


// the resistance thresholds when using 22M ohm resistors
const int PressedMaxThreshold = 200; // this is the maximum reading when the button is pressed. Anything underneath this value will register as a touch (a higher number will allow inputs with a higher natural resistance at the risk of false positives)
const int ReleasedMinThreshold = 300; // this is the minimum reading when there is no connection. Any reading higher than this will register as no longer touching
const byte PinCount = 4;
unsigned long inputDelay = 500;      // the amount of time to delay before processing the event
// const unsigned long maxTime = 3456000000;       // the maximum amount of time (40 days). Basically infinity

// you can have as many keys as you have analog pins
const byte InputPins[PinCount] = {A0, A1, A2, A3};
// how to program arbitrary letters
const char KeyCodes[PinCount] = {'w', 'a', 's','d'};
// how to use special keys
//const char KeyCodes[PinCount] = {KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_LEFT_ARROW, KEY_RIGHT_ARROW};


// every character/key has these attributes
struct TouchInput {
    byte analogPin;
    char keycode;
    movingAvg filter = movingAvg(20);
    boolean wasPressed = false;
};



// so instead of pressing keys when I detect them, I should add them to a queue of events which I process after a delay
struct Event {
    char keycode;   // what letter to press. Not sure how to do this with joystick input...
    bool pressButton;     // whether to press or release the key
    unsigned long timeToExecute;    // the time the action should be executed
};

// this is an array of TouchInput structs called "Pins" that contains "PinCount" items in it
TouchInput Pins[PinCount];

// const int eventBuffer = 20;       // I need a finite number of events to process. 20 seems good...
// Event events[eventBuffer];
List<Event> eventList;


void setup(){
    Serial.begin(115200);

    // loop through each of the keys and initialize them in the order defined in InputPins[] KeyCodes[]
    for(int i = 0; i < PinCount; i++){
        Pins[i].analogPin = InputPins[i];
        Pins[i].keycode = KeyCodes[i];
        Pins[i].filter.begin();
    }

    Serial.println("In setup()");

    

    // for(int i = 0; i < eventBuffer; i++){
    //     events[i].keycode = '-';
    //     events[i].action = 0;
    //     // this is 40 days worth of milliseconds. Basically infinity
    //     events[i].timeToExecute = maxTime;
    // }
}


void loop(){
    // Serial.println("test");
    // delay(300);
    // evaluate each of the pins defined earlier
    for(int i = 0; i < PinCount; i++){

        // receive button inputs:
        float currentAverage = Pins[i].filter.reading(analogRead(Pins[i].analogPin));
        // TODO: it's probably not efficient to initialize these every time...
        boolean previousState = Pins[i].wasPressed;
        boolean currentState = previousState;   // Default if in the dead zone

        if(currentAverage < PressedMaxThreshold){
            currentState = true;        // this means that the circuit has been completed, so the key should be pressed
        }
        else if(currentAverage > ReleasedMinThreshold){
            currentState = false;       // the circuit has been broken, so the key should be released
        }

        // if the state of the pin has changed since the last iteration
        if(currentState != previousState){
            // then start or stop pressing the key (the opposite of what it was before)
            // if(currentState){
            //     Keyboard.press(Pins[i].keycode);
            // }else{
            //     Keyboard.release(Pins[i].keycode);
            // }

            // create a new event and add it to the list
            // the time that the event should be processed is calculated here
            Serial.println("Creating new event to " + String(currentState) + " the letter " + Pins[i].keycode);
            Event newEvent = {
                Pins[i].keycode,
                currentState,
                millis() + inputDelay
            };
            eventList.Add(newEvent);     
        }
        Pins[i].wasPressed = currentState;
    }

    // process events in the event list
    if(!eventList.IsEmpty()){
        // cycle through all the events in the list and process them
        for(int i = 0; i < eventList.Count(); i++){

            // if it's time to process the event
            if(eventList[i].timeToExecute <= millis()){
                // if the action associated with this event is a press, press the key
                if(eventList[i].pressButton){
                    Serial.println("Time has elapsed, pressing " + String(eventList[i].keycode));
                    Keyboard.press(eventList[i].keycode);
                }else{
                    // otherwise, release the key
                    Serial.println("Time has elapsed, releasing " + String(eventList[i].keycode));
                    Keyboard.release(eventList[i].keycode);
                }
                // the event has been processed, remove it from the list
                eventList.Remove(i);
            }
            // else{
            //     // the events will all be in order from soonest to latest, so if it's not time to process the current element, none of the subsequent elements will be ready yet either
            //     break;
            // }
        }
    }
}

