# Default build - typically debug or unoptimized build
build:
	pio run -e esp32dev

# Release build - optimized and for production use
release:
	pio run -e release

# Package firmware binaries for distribution under a specific directory
package: release
	@echo "Packaging release binaries..."
	@if [ ! -d "release_to_manufacture/release_package" ]; then \
	    echo "Creating directory release_to_manufacture/release_package"; \
	    mkdir -p release_to_manufacture/release_package; \
	else \
	    echo "Directory release_to_manufacture/release_package already exists"; \
	fi
	cp .pio/build/release/bootloader.bin release_to_manufacture/release_package/
	cp .pio/build/release/firmware.bin release_to_manufacture/release_package/
	cp .pio/build/release/partitions.bin release_to_manufacture/release_package/
	@echo "Creating zip file in release_to_manufacture directory..."
	cd release_to_manufacture && zip -r release_package.zip release_package
	@echo "Package ready at release_to_manufacture/release_package.zip"

# Install firmware to the device
install:
	pio run -e esp32dev --target upload

# Install release firmware to the device
install-release:
	pio run -e release --target upload

# Install firmware and then open the serial monitor
install-monitor:
	pio run -e esp32dev --target upload --target monitor

# Open the serial monitor
monitor:
	pio device monitor

# Clean up the build (remove compiled files)
clean:
	pio run --silent -t clean
	rm -f release_to_manufacture/release_package/*.bin  # Remove only binary files from the package directory
	rm -f release_to_manufacture/release_package.zip    # Remove the zip file

# Show help for SPIFFS tool
spiffs:
	pio pkg exec --package "platformio/tool-mkspiffs" -- -h
