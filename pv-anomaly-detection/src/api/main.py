from fastapi import FastAPI
from pydantic import BaseModel
from src.models.inference import predict_anomaly

app = FastAPI(title="PV Anomaly Detection API")

class Measurement(BaseModel):
    timestamp: str       # "2024-12-01 12:00:00"
    temperature: float   # deg C
    humidity: float      # %
    power_mW: float      # mW

class AnomalyResponse(BaseModel):
    is_anomaly: bool
    anomaly_score: float
    reason: str

@app.get("/")
def root():
    return {"message": "PV anomaly detection API is running."}

@app.post("/predict", response_model=AnomalyResponse)
def predict(measurement: Measurement):
    result = predict_anomaly(
        timestamp=measurement.timestamp,
        temperature=measurement.temperature,
        humidity=measurement.humidity,
        power_mW=measurement.power_mW,
    )
    return AnomalyResponse(**result)
