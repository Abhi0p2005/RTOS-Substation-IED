# System Architecture

## Overview

The IED Protection System is built on a multi-layered architecture designed for real-time protection relay functionality with FreeRTOS.

## Layer Architecture

```
┌─────────────────────────────────────────────────┐
│            Application Layer                    │
│  ┌──────────────┐  ┌──────────────┐             │
│  │  Protection  │  │     HMI      │             │
│  │   Logic      │  │  Interface   │             │
│  └──────────────┘  └──────────────┘             │
└─────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────┐
│         Algorithm Layer                         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐       │
│  │ ANSI 50  │  │ ANSI 51  │  │   RMS    │       │
│  │Instant OC│  │ Time OC  │  │Calculation│      │
│  └──────────┘  └──────────┘  └──────────┘       │
└─────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────┐
│            HAL Layer                            │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐       │
│  │   ADC    │  │  Relay   │  │   LCD    │       │
│  │ Sampling │  │ Control  │  │ Display  │       │
│  └──────────┘  └──────────┘  └──────────┘       │
└─────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────┐
│         FreeRTOS / ESP32                        │
└─────────────────────────────────────────────────┘
```

## Task Structure

### Priority Levels

- **Priority 4** - ADC Task (Highest)
- **Priority 3** - RMS Task
- **Priority 2** - Protection Task
- **Priority 1** - HMI Task (Lowest)

### Data Flow

```
ADC Task (6400 Hz) 
    ↓ [Queue: 32 samples]
RMS Task (processes per cycle)
    ↓ [Queue: 8 RMS results]
Protection Task (evaluates trip)
    ↓ [Relay control]
Trip FSM + Relay Output
```

## Protection Logic

### ANSI 50 - Instantaneous Overcurrent

```
Input: Ia, Ib, Ic (RMS)
  ↓
Max(Ia, Ib, Ic) >= Pickup?
  ↓ YES
Time since pickup >= Delay?
  ↓ YES
TRIP (Instantaneous)
```

### ANSI 51 - Time Overcurrent (IDMT)

```
Input: Ia, Ib, Ic (RMS)
  ↓
Current Multiple M = I / I_pickup
  ↓
M >= 1.0?
  ↓ YES
Calculate trip time: t = (k × TMS) / (M^α - β)
  ↓
Elapsed >= trip time?
  ↓ YES
TRIP (Time delayed)
```

## Sampling Strategy

**System Frequency**: 50 Hz
**Samples per Cycle**: 128
**Sampling Rate**: 6400 Hz (50 × 128)
**RMS Window**: 1 cycle (128 samples)
**RMS Update Rate**: 50 Hz (once per cycle)

## State Machine

```
       ┌──────┐
       │ IDLE │◄────────────┐
       └──┬───┘             │
          │ Fault Detected  │
          ↓                 │
      ┌────────┐            │
      │ PICKUP │            │
      └───┬────┘            │
          │ Trip Time       │
          ↓                 │
       ┌──────┐   Reset     │
       │ TRIP ├─────────────┘
       └──────┘
```

## Configuration

Runtime configurable parameters:

- ANSI 50: Pickup current, delay time, enable/disable
- ANSI 51: Pickup current, TMS, curve type, enable/disable
- System: Rated current, frequency

## Platform Abstraction

### Native Simulator

- Software-generated 50Hz waveforms
- Fault injection via commands
- Console-based HMI

### ESP32 Hardware

- ADC1 channels for CT inputs
- GPIO for relay control
- NVS for configuration storage
- Optional I2C LCD display

## Safety Features

1. **Priority inversion prevention**: High-priority ADC task
2. **Watchdog protection**: FreeRTOS task monitoring
3. **Trip verification**: FSM prevents spurious trips
4. **Manual reset**: Latching trip requiring operator intervention
5. **Event logging**: Trip history with timestamp and cause
