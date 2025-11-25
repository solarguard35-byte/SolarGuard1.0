import joblib
import pandas as pd
from pathlib import Path
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import Pipeline

PROCESSED_DATA_PATH = Path("data/processed/pv_clean.csv")
MODEL_PATH = Path("models/isolation_forest.joblib")

def train_isolation_forest(contamination=0.01):
    df = pd.read_csv(PROCESSED_DATA_PATH)
    feature_cols = [
        "Temperature(oC)",
        "Humidity(%)",
        "P_rel",
        "time_sin",
        "time_cos",
    ]

    X = df[feature_cols].values

    # Pipeline: scaling + IsolationForest
    pipe = Pipeline([
        ("scaler", StandardScaler()),
        ("model", IsolationForest(
            n_estimators=200,
            contamination=contamination,  # 3% anomalies
            random_state=42
        )),
    ])

    pipe.fit(X)

    MODEL_PATH.parent.mkdir(parents=True, exist_ok=True)
    joblib.dump(
        {"pipeline": pipe, "features": feature_cols},
        MODEL_PATH
    )

    print("Model trained and saved to", MODEL_PATH)

if __name__ == "__main__":
    train_isolation_forest()
