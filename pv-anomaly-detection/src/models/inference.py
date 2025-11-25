import joblib
import numpy as np
import pandas as pd
from pathlib import Path
from datetime import datetime

MODEL_PATH = Path("models/isolation_forest.joblib")
PANEL_P_NOM = 1.2  # W

_cached = None

def load_model():
    global _cached
    if _cached is None:
        _cached = joblib.load(MODEL_PATH)
    return _cached["pipeline"], _cached["features"]

def build_features(timestamp: str, temperature: float,
                   humidity: float, power_mW: float):
    """
    timestamp: ISO string 'YYYY-MM-DD HH:MM:SS'
    """
    dt = pd.to_datetime(timestamp)

    power_W = power_mW / 1000.0
    p_rel = power_W / PANEL_P_NOM

    seconds_since_midnight = (
        dt.hour * 3600 + dt.minute * 60 + dt.second
    )
    seconds_in_day = 24 * 3600
    time_frac = seconds_since_midnight / seconds_in_day

    time_sin = np.sin(2 * np.pi * time_frac)
    time_cos = np.cos(2 * np.pi * time_frac)

    return np.array([[temperature, humidity, p_rel, time_sin, time_cos]])

def predict_anomaly(timestamp: str, temperature: float,
                    humidity: float, power_mW: float):
    pipe, feature_cols = load_model()
    X = build_features(timestamp, temperature, humidity, power_mW)

    # === Model decision ===
    y_pred = pipe.predict(X)[0]          # 1 or -1
    score = pipe.decision_function(X)[0]
    raw_is_anomaly = (y_pred == -1)

    # keep only strong model anomalies
    model_anomaly = raw_is_anomaly and (score < -0.05)

    # === Rule-based checks ===
    reasons = []
    rule_anomaly = False

    dt = pd.to_datetime(timestamp)

    power_W = power_mW / 1000.0
    p_rel = power_W / PANEL_P_NOM

    # 1. Physical limits
    if temperature < 0 or temperature > 70:
        reasons.append("Temperature out of physical range.")
        rule_anomaly = True

    if humidity < 0 or humidity > 100:
        reasons.append("Humidity value is invalid.")
        rule_anomaly = True

    if p_rel > 1.2:  # >120% nominal
        reasons.append("Power exceeds physical panel limit.")
        rule_anomaly = True

    # 2. Night-time behavior
    if dt.hour < 6 or dt.hour > 19:
        if power_W > 0.1:
            reasons.append("Power detected during night time.")
            rule_anomaly = True

    # 3. Daylight underperformance  (THIS IS YOUR CASE)
    if 10 <= dt.hour <= 16:  # peak sunlight
        if temperature > 20 and p_rel < 0.05:
            reasons.append("Power too low for daylight and temperature.")
            rule_anomaly = True   # <-- FORCE anomaly by rule

    # === Combine model + rules ===
    is_anomaly = model_anomaly or rule_anomaly

    # Add model-based reason if it contributed
    if model_anomaly:
        reasons.append("Model detected abnormal behavior pattern.")

    # If still no reasons, it's normal
    if not reasons:
        reasons.append("Normal operation.")

    return {
        "is_anomaly": bool(is_anomaly),
        "anomaly_score": float(score),
        "reason": " | ".join(reasons)
    }

if __name__ == "__main__":
    # Quick manual test
    res = predict_anomaly("2024-12-01 12:00:00",
                          temperature=35,
                          humidity=60,
                          power_mW=500)
    print(res)
