# Build configuration

## Local configuration

Copy `els2/els_wifi_config.example.h` to `els2/els_wifi_config.h`. In the copy,
replace the template string with your chosen 8–63 character Wi-Fi passphrase,
keeping the straight double quotes. Use a plain-text editor or Arduino IDE.
On Windows, show extensions and check the file is not accidentally named `.h.txt`.

Git ignores this local header. The setting takes effect after uploading the new
firmware; reconnect using the configured value. Compiled binaries contain this setting.

## Optional command-line build

Install [Arduino CLI](https://arduino.github.io/arduino-cli/latest/installation/).
From the repository root, after creating the local header, run these single-line
commands in macOS Terminal or Windows PowerShell:

```sh
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32@3.3.11
arduino-cli lib install "WebSockets@2.7.2" "U8g2@2.36.19"
arduino-cli compile --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=default_8MB,FlashSize=8M --export-binaries els2/
```

For USB upload, use `arduino-cli board list` and substitute the actual port below:

```sh
arduino-cli upload --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=default_8MB,FlashSize=8M -p PORT els2/
```

Windows ports normally resemble `COM5`; macOS native USB ports resemble
`/dev/cu.usbmodem…`. Use the full port returned by your system.
