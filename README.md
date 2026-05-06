# ReLift Fall Recovery System
Assistive technology system for fall response using wearable sensing and wireless actuation with built-in safety mechanisms.

## Overview

ReLift is a modular assistive system designed to reduce prolonged lie times following ground-level falls, particularly in elderly or mobility-impaired individuals. The system integrates a wearable sensing unit with a walker-mounted recovery platform to enable rapid response to detected events.

The project focuses on **embedded system integration, wireless communication, and real-time actuation control**, demonstrating how distributed devices can coordinate safely in assistive applications.

> ⚠️ Note: Portions of this repository have been intentionally abstracted to protect intellectual property related to a pending patent. The code provided demonstrates system architecture and engineering implementation, but does not expose proprietary detection or decision-making algorithms. This repository does not represent a finished or clinically validated medical device.

---

## Development Context

This project was developed as part of a team-based engineering design initiative.

My primary contributions included:

* Development of the embedded software for both the wearable and platform modules
* Implementation of BLE communication between distributed devices
* Design of the platform control logic, including state-based deployment behavior and safety mechanisms
* Integration of sensor data processing with real-time system response

The project was completed collaboratively, with contributions spanning mechanical design, system integration, and testing.

---

## System Architecture

The ReLift system consists of two primary modules:

### 1. Wearable Module

* Embedded IMU-based sensing unit
* Performs real-time motion analysis
* Communicates wirelessly via Bluetooth Low Energy (BLE)
* Acts as a **BLE server**

### 2. Platform Module (Walker-Attached)

* Receives signals from the wearable
* Executes deployment of a recovery mechanism
* Includes safety features such as:

  * Warning phase prior to actuation
  * User override functionality
* Acts as a **BLE client**

### Communication Flow

Wearable → detects critical event → sends signal → Platform → validates → warns user → deploys if not overridden

---

## Key Features

### Embedded Sensing & Processing

* Real-time IMU data acquisition (accelerometer + gyroscope)
* On-device motion signal handling (implementation abstracted)
* Motion state estimation (high-level)

### Wireless Communication (BLE)

* Custom BLE service architecture
* Bidirectional communication between modules
* Connection management with timeout handling and reconnection logic

### Platform Actuation Logic

* Servo-based mechanical actuation
* Multi-stage activation process:

  * Trigger event detection (abstracted)
  * Pre-action warning phase
  * Conditional actuation step
* Manual override with debounce handling

### Fault Tolerance & Robustness

* Connection timeout detection
* Automatic reconnection attempts
* State reset mechanisms after deployment
* Continuous system status updates

---

## Repository Structure

```
ReLift-System/
│
├── platform_module/
│   └── platform_main_redacted.ino
│
├── wearable_module/
│   └── wearable_main_redacted.ino
│
├── docs/
│   └── system_overview.md
│
└── README.md
```

---

## Technologies Used

* **Microcontroller:** ESP32
* **Sensors:** MPU6050 (6-axis IMU)
* **Communication:** Bluetooth Low Energy (BLE)
* **Actuation:** Servo motor control
* **Programming Language:** C++ (Arduino framework)

---

## Design Highlights

### Distributed Embedded System

The system is split across two independent devices that must:

* Maintain a reliable wireless connection
* Coordinate behavior in real time
* Handle asynchronous events safely

### State Machine-Based Control

The platform module uses a structured state-based approach to manage:

* Idle operation
* Pre-deployment warning
* Deployment execution

This ensures predictable and safe behavior under varying conditions.

### Human-in-the-Loop Safety Design

A key design priority is **preventing unintended deployment**. The system incorporates:

* A timed warning phase with user feedback
* A physical override mechanism
* Safety-gated activation logic (abstracted at implementation level)

---

## Limitations

To support intellectual property protection, certain implementation details have been intentionally omitted or generalized, including:

* Internal decision logic
* Parameter tuning values
* Event classification strategy

The repository focuses on embedded system integration, communication design, and actuation control, rather than internal decision logic.

---

## Future Work

* Integration with emergency alert systems
* Enhanced power optimization for wearable module
* Mechanical optimization of deployment platform
* Clinical validation and usability testing

---

## Author

**Tobi Abiola-Lawal**
Biomedical Engineering Student, University of Calgary

Interested in:

* Assistive technologies
* Embedded systems
* Sensor integration
* Human-centered design

---

## Disclaimer

This repository is intended for demonstration and educational purposes. It showcases system design and embedded implementation but does not include proprietary algorithms or full system capabilities.
