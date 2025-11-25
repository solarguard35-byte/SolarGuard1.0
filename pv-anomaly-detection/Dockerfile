FROM python:3.11-slim

WORKDIR /app

# Install OS deps if needed (optional)
RUN pip install --no-cache-dir --upgrade pip

# Copy requirements & install
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.prod.txt

# Copy code and model
COPY src ./src
COPY models ./models

ENV PYTHONPATH=/app

EXPOSE 8000

CMD ["uvicorn", "src.api.main:app", "--host", "0.0.0.0", "--port", "8000"]
