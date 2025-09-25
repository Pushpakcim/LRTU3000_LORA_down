# LoRa Downlink Check Implementation

This document describes the enhanced LoRa downlink checking functionality implemented to verify cloud connectivity and monitor downlink message reception.

## Overview

The LRTU3000 device now includes comprehensive downlink monitoring and diagnostics for LoRa cloud communication. This enhancement provides real-time logging, periodic diagnostics, and error detection for downlink messages from the cloud server.

## Features

### 1. Real-time Downlink Logging
- **AT+RECV Command Logging**: Every `AT+RECV=<port>,<hex_data>` command is logged with detailed information
- **RX Event Logging**: All `+EVT:RX_C:` events are logged with RSSI, SNR, and signal quality data
- **Data Validation**: Hex data is validated before conversion and processing
- **Buffer Status**: ModBus buffer size and processing status are logged

### 2. Periodic Diagnostics
- **2-minute intervals**: Comprehensive diagnostics run every 120 seconds
- **Network status**: Join state, network mode, device class, and region information
- **Signal quality**: RSSI and SNR monitoring with threshold warnings
- **Device information**: EUI, firmware version, and configuration details

### 3. Error Detection and Protection
- **Buffer overflow protection**: Prevents data corruption with logging
- **Empty data detection**: Identifies and logs empty downlink messages
- **Invalid port detection**: Logs invalid port numbers in downlink commands
- **Signal quality warnings**: Alerts when RSSI drops below -120 dBm

## Implementation Details

### Enhanced Functions

#### `parse_lora_rx(const char* str)`
- Validates downlink data before processing
- Logs raw hex data and converted ASCII data
- Updates ModBus buffer with comprehensive logging
- Sets reception status flags

#### `parse_lora_rx_event(const char* str)`
- Extracts and logs signal quality metrics (RSSI, SNR)
- Logs message type and port information
- Confirms successful event processing

#### `check_lora_downlink_status(void)`
- Returns 1 if downlink is functional, 0 if issues detected
- Checks network join state
- Monitors signal quality
- Verifies device class configuration
- Reports recent downlink activity

#### `log_lora_downlink_diagnostics(void)`
- Comprehensive system status logging
- Network configuration details
- Signal quality metrics
- Device identification information
- Buffer and reception status

### Logging Format

All downlink-related logs use the prefix `[LORA_DOWNLINK]` or `[LORA_DIAGNOSTICS]` for easy identification:

```
[LORA_DOWNLINK] Raw AT command: AT+RECV=130,030001000902010300a100141427
[LORA_DOWNLINK] AT+RECV received, port=130
[LORA_DOWNLINK] Raw hex data: 030001000902010300a100141427 (len=28)
[LORA_DOWNLINK] Success: Data processed, ModBus buffer size=14
```

## Usage

### Automatic Operation
The downlink checking operates automatically once the system is running:

1. **Real-time monitoring**: All downlink messages are automatically logged
2. **Periodic diagnostics**: Status checks run every 2 minutes
3. **Error reporting**: Issues are automatically detected and logged

### Manual Diagnostics
To manually trigger diagnostics, call these functions from your code:

```c
// Check current downlink status
uint8_t status = check_lora_downlink_status();
if (status == 0) {
    // Handle downlink issues
}

// Log comprehensive diagnostics
log_lora_downlink_diagnostics();
```

### Log Monitoring
Monitor the system logs for these key indicators:

**Success Indicators:**
- `[LORA_DOWNLINK] Success: Data processed`
- `[LORA_DOWNLINK_CHECK] OK: Joined to LoRa network`
- `[LORA_TASK] INFO: Downlink status check passed`

**Warning Indicators:**
- `[LORA_DOWNLINK_CHECK] WARNING: Poor signal quality`
- `[LORA_DOWNLINK_CHECK] WARNING: Not in Class C mode`
- `[LORA_DOWNLINK] ERROR: Empty hex data received`

**Error Indicators:**
- `[LORA_DOWNLINK_CHECK] ERROR: Not joined to LoRa network`
- `[LORA_DOWNLINK] ERROR: RX buffer overflow`
- `[LORA_TASK] WARNING: Downlink status check indicates issues`

## Troubleshooting

### Common Issues

1. **No Downlink Messages**
   - Check network join status: `Network Join State: 1` (should be 1)
   - Verify Class C mode: `Device Class: C`
   - Check signal quality: `RSSI: -XX dBm` (should be > -120)

2. **Poor Signal Quality**
   - Monitor RSSI values in diagnostics
   - Consider antenna placement or gateway proximity
   - Check for interference

3. **Buffer Overflow**
   - Monitor message size and frequency
   - Check for corrupted or oversized messages
   - Verify hex data format

### Diagnostic Schedule
- **Real-time**: Every downlink message
- **Periodic**: Every 2 minutes (120 seconds)
- **On-demand**: Call diagnostic functions manually

## Configuration

The downlink checking uses these configurable parameters:

- **Diagnostic interval**: 120 seconds (modifiable in `RAK_Lora.c`)
- **RSSI threshold**: -120 dBm (modifiable in `check_lora_downlink_status()`)
- **Buffer sizes**: 250 bytes for hex, 256 bytes for ASCII
- **Log buffer**: 500 characters for debug messages

## Integration

The downlink checking integrates with:
- **ModBus protocol**: Processed downlink data flows to ModBus handlers
- **Logging system**: Uses existing `WriteLog()` function
- **LoRa task**: Periodic checks in main LoRa task loop
- **Network monitoring**: Coordinates with existing network status checks

## Files Modified

- `Core/Inc/RxRingProcess.h`: Added function declarations
- `Core/Src/RxRingProcess.c`: Enhanced parsing functions and added diagnostics
- `Core/Src/RAK_Lora.c`: Added periodic checking and enhanced logging