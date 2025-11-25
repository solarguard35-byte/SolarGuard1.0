# SolarGuard ⚡

> AI-powered solar panel monitoring system with anomaly detection

---

## 🌟 Overview

**SolarGuard** monitors photovoltaic (PV) panel performance by analyzing **temperature**, **humidity**, and **power output** to detect anomalies before they become critical issues.

---

## 📂 Project Structure

```
solarguard/
│
├── nextjs-pv-anomaly/       # Next.js AI application
├── pv-anomaly-detection/    # Python ML model
├── pv-frontend/             # Web dashboard
└── test.py                  # Testing script
```

---

## ✨ Features

✅ **Real-time PV monitoring**  
✅ **AI-based anomaly detection**  
✅ **Historical data analysis**  
✅ **Interactive charts**  
✅ **CSV export**  
✅ **Severity classification** (Critical, High, Medium, Normal)

---

## 🛠️ Prerequisites

- **Python** 3.8+
- **Node.js** 18+
- **Modern web browser**

---

## 📥 Installation

### 1. Clone Repository
```bash
git clone https://github.com/solarguard35-byte/solarguard.git
cd solarguard
```

### 2. Setup Python Model
```bash
cd pv-anomaly-detection
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
```

### 3. Setup Next.js Application
```bash
cd nextjs-pv-anomaly
npm install
npm run dev
```

### 4. Setup Frontend
```bash
cd pv-frontend
# Open index.html in browser or use local server
```

---

## 🚀 Usage

### Login Credentials
| Field | Value |
|-------|-------|
| **Username** | `admin` |
| **Password** | `admin123` |

### Running the System

1. **Start Next.js backend:**
   ```bash
   npm run dev
   ```
   > Runs on http://localhost:3000

2. **Open frontend:** Launch `pv-frontend/index.html` in browser

3. **Login and explore** the dashboard

---

## 🧰 Technology Stack

| Category | Technologies |
|----------|-------------|
| **Backend** | Python, scikit-learn, pandas, numpy |
| **Framework** | Next.js |
| **Frontend** | HTML5, CSS3, JavaScript |
| **Visualization** | Chart.js |


---

### ⭐ Star this repo if you find it useful!
