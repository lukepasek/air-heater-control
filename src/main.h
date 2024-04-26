void initSerialBitTimer();
void restartSerialBitTimer();
void stopSerialBitTimer();
void attachSerialPinEdgeInterrupt();
void detachSerialPinEdgeInterrupt();

void serialPinEdgeHandler();
void serialBitTimerHandler();

void led_on(bool);