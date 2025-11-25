import pandas as pd
import numpy as np
from pathlib import Path

RAW_DATA_PATH = Path("data/raw/final_data_daaaata.csv")
PROCESSED_DATA_PATH = Path("data/processed/pv_clean.csv")

PANEL_V_NOM = 4.0      # volts
PANEL_I_NOM = 0.3      # amps
PANEL_P_NOM = PANEL_V_NOM * PANEL_I_NOM  # 1.2 W

def load_and_preprocess():
    df = pd.read_csv(RAW_DATA_PATH)

    # Parse datetime
    df["Date Time"] = pd.to_datetime(df["Date Time"])
    df = df.sort_values("Date Time")

    # Convert power to watts
    df["Power_W"] = df["Power(mW)"] / 1000.0

    # Relative power (0..1+)
    df["P_rel"] = df["Power_W"] / PANEL_P_NOM

    # Time features (time of day)
    df["hour"] = df["Date Time"].dt.hour
    df["minute"] = df["Date Time"].dt.minute
    df["second"] = df["Date Time"].dt.second

    df["seconds_since_midnight"] = (
        df["hour"] * 3600 + df["minute"] * 60 + df["second"]
    )
    seconds_in_day = 24 * 3600
    df["time_frac"] = df["seconds_since_midnight"] / seconds_in_day

    # Circular encoding for time-of-day
    df["time_sin"] = np.sin(2 * np.pi * df["time_frac"])
    df["time_cos"] = np.cos(2 * np.pi * df["time_frac"])

    # Simple cleaning: drop clearly wrong rows (optional)
    df = df[
        (df["Temperature(oC)"] > 0) &
        (df["Humidity(%)"] > 0) &
        (df["Humidity(%)"] <= 100)
    ]

    # Keep only columns we will use for the model
    feature_cols = [
        "Temperature(oC)",
        "Humidity(%)",
        "P_rel",
        "time_sin",
        "time_cos",
    ]

    df_features = df[["Date Time"] + feature_cols].copy()
    PROCESSED_DATA_PATH.parent.mkdir(parents=True, exist_ok=True)
    df_features.to_csv(PROCESSED_DATA_PATH, index=False)

    return df_features, feature_cols


if __name__ == "__main__":
    df_features, feature_cols = load_and_preprocess()
    print("Saved processed data to:", PROCESSED_DATA_PATH)
    print("Features:", feature_cols)
