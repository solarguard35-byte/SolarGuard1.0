# SolarGuard ⚡

> **AI-powered solar tracking & monitoring system with ESP32, MQTT, and anomaly detection**

---

## 🌟 Overview

**SolarGuard** is an intelligent IoT-based solar monitoring and tracking platform combining:

* **ESP32 firmware** for real-time solar tracking & sensor acquisition
* **MQTT cloud communication**
* **Next.js dashboard** for visualization
* **AI anomaly detection** using Python models

It monitors **temperature**, **humidity**, **voltage**, **current**, and **power output**, while applying **AI-based analytics** to detect anomalies before they become critical issues.

---

## 🔧 ESP32 Tracking Firmware (New!)

This ESP32-based firmware powers the SolarGuard intelligent solar tracking system, implementing fully autonomous sun-following capability through **dual LDR (Light Dependent Resistor) sensors** that continuously measure light intensity on opposite sides of the solar panel.

The system reads both LDR values every **500ms** and automatically adjusts a **servo motor** to minimize the difference between them:

* When the **top LDR** detects more light → the panel tilts **upward**
* When the **bottom LDR** receives more light → it tilts **downward**

This creates a **self-correcting feedback loop** that keeps the panel aligned perpendicularly with the sun **without using GPS, RTC, or complex solar position algorithms**.

The firmware also integrates:

* **DHT11** → temperature & humidity
* **ACS712** → current measurement
* **Voltage divider** → voltage sensing
* **Power calculation** → `P = V × I`
* **Tracking accuracy metrics**

Every **3 seconds**, all data points are packaged into **JSON** and published to a cloud MQTT broker, including:

* Temperature & humidity
* Voltage, current & computed power
* LDR values
* Servo angle
* System status

The web dashboard (React/Next.js) uses this stream to provide:

* Real-time charts
* AI-powered anomaly detection
* Predictive maintenance alerts
* Remote control
* Mode switching (Auto tracking ↔ Manual positioning)

This fully connected workflow creates an **IoT-enabled smart solar system optimized for Tunisia’s high-irradiance climate**.

---

## 📂 Project Structure

```
solarguard/
│
├── esp32-firmware/          # NEW: ESP32 solar tracking + MQTT firmware
├── nextjs-pv-anomaly/       # Next.js AI application
├── pv-anomaly-detection/    # Python ML model
├── pv-frontend/             # Web dashboard
└── test.py                  # Testing script
```

---

## ✨ Features

### 🔭 Hardware & IoT

* Autonomous dual-LDR sun tracking
* Servo-based solar panel rotation
* Temperature & humidity sensing
* Real-time voltage & current monitoring
* Power computation & efficiency analysis
* MQTT telemetry every 3 seconds
* Remote control & mode switching

### 🧠 AI & Software

* Real-time PV monitoring
* AI-based anomaly detection
* Historical data analysis
* Interactive charts
* CSV export
* Severity classification (Critical, High, Medium, Normal)

---

## 🛠️ Prerequisites

* **Python** 3.8+
* **Node.js** 18+
* **ESP32 Dev Board**
* **MQTT Broker** (Cloud or local)
* **Modern web browser**

---

## 📥 Installation

### 1. Clone Repository

```bash
git clone https://github.com/solarguard35-byte/solarguard.git
cd solarguard
```

### 2. Setup ESP32 Firmware

```bash
cd esp32-firmware
# open in Arduino IDE or PlatformIO
```

### 3. Setup Python Model

```bash
cd pv-anomaly-detection
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
```

### 4. Setup Next.js Application

```bash
cd nextjs-pv-anomaly
npm install
npm run dev
```

### 5. Setup Frontend

```bash
cd pv-frontend
# Open index.html in browser or use a local server
```

---

## 🚀 Usage

### Login Credentials

| Field    | Value      |
| -------- | ---------- |
| Username | `admin`    |
| Password | `admin123` |

### Running the System

1. Start Next.js backend:

   ```bash
   npm run dev
   ```

   > Runs on [http://localhost:3000](http://localhost:3000)

2. Open the dashboard:
   Open `pv-frontend/index.html`

3. Log in and monitor the system

---

## 🧰 Technology Stack

| Category          | Technologies                        |
| ----------------- | ----------------------------------- |
| **IoT Hardware**  | ESP32, DHT11, ACS712, LDRs, Servo   |
| **Protocols**     | MQTT                                |
| **Backend**       | Python, scikit-learn, pandas, numpy |
| **Framework**     | Next.js                             |
| **Frontend**      | HTML5, CSS3, JavaScript             |
| **Visualization** | Chart.js                            |

---

If you want, I can also:

✔️ Generate a **diagram** showing the architecture
✔️ Write a **full documentation** section for the ESP32 firmware
✔️ Generate **badges (shields.io)** for GitHub
✔️ Create a **logo** for SolarGuard
✔️ Create **architecture flowcharts**