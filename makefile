build:
	pio run -e esp32dev

install:
	pio run -e esp32dev --target upload

monitor:
	pio device monitor

clean:
	pio run --clean

spiffs:
	pio pkg exec --package "plaformio/tool-mkspiffs" -- -h
