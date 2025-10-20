

DEVICE = /dev/ttyACM0

SKETCH = AVR.ino

BAUDRATE = 115200


build:
	arduino-cli compile --fqbn arduino:avr:uno $(SKETCH)

upload:
	arduino-cli upload -p $(DEVICE) --fqbn arduino:avr:uno $(SKETCH)

monitor:
	arduino-cli monitor -p $(DEVICE) -c $(BAUDRATE)


test: build upload monitor





