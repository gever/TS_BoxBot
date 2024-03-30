build:
	pio run -e esp32dev

install:
	pio run -e esp32dev --target upload

install-monitor:
	pio run -e esp32dev --target upload --target monitor

monitor:
	pio device monitor

clean:
	pio run --clean

spiffs:
	pio pkg exec --package "plaformio/tool-mkspiffs" -- -h
