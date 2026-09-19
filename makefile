# Default build - typically debug or unoptimized build
build:
	pio run -e esp32dev

# Build the release version of the firmware
release:
	pio run -e release

# Build the SPIFFS image for the release environment
buildfs-release:
	pio run -e release -t buildfs

# Build the SPIFFS image for the development environment
buildfs-dev:
	pio run -e esp32dev -t buildfs

# Combined target to build both the firmware and SPIFFS image for release
all-release:
	pio run -e release
	pio run -e release -t buildfs

# Combined target to build both the firmware and SPIFFS image for development
all-dev:
	pio run -e esp32dev
	pio run -e esp32dev -t buildfs

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
	cp .pio/build/release/spiffs.bin release_to_manufacture/release_package/
	@echo "Creating zip file in release_to_manufacture directory..."
	cd release_to_manufacture && zip -r release_package.zip release_package
	@echo "Package ready at release_to_manufacture/release_package.zip"

# Copy release binaries into docs/firmware/ for the ESP Web Tools GitHub Pages installer.
# Run this after `make package` whenever you cut a new release.
publish-docs: package
	@echo "Copying release binaries to docs/firmware/..."
	@mkdir -p docs/firmware
	cp .pio/build/release/bootloader.bin docs/firmware/
	cp .pio/build/release/firmware.bin   docs/firmware/
	cp .pio/build/release/partitions.bin docs/firmware/
	cp .pio/build/release/spiffs.bin     docs/firmware/
	@echo "docs/firmware/ updated. Commit and push to update the web installer."

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
