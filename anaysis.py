import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.preprocessing import StandardScaler
from sklearn.ensemble import IsolationForest, RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, confusion_matrix
from xgboost import XGBClassifier
import joblib

# Step 1: Load and preprocess data
df = pd.read_csv('sensor_data.csv', parse_dates=['timestamp'])

# Step 2: Handle missing values
df.fillna(method='ffill', inplace=True)

# Step 3: Feature Engineering
df['hour'] = df['timestamp'].dt.hour
df['is_day'] = ((df['hour'] > 6) & (df['hour'] < 18)).astype(int)

# Step 4: Label data for training
df['unsafe'] = ((df['temperature'] > 35) | (df['air_quality'] > 600)).astype(int)

# Step 5: Scale features
features = ['temperature', 'humidity', 'light_level', 'air_quality', 'hour', 'is_day']
X = df[features]
y = df['unsafe']

scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

# Step 6: Anomaly Detection (Isolation Forest)
iso = IsolationForest(contamination=0.1)
anomaly_flags = iso.fit_predict(X_scaled)
df['anomaly'] = (anomaly_flags == -1).astype(int)

# Step 7: ML Model - XGBoost Classifier
X_train, X_test, y_train, y_test = train_test_split(X_scaled, y, test_size=0.2, random_state=42)
model = XGBClassifier()
model.fit(X_train, y_train)
y_pred = model.predict(X_test)

print("\n📊 Classification Report (XGBoost):")
print(classification_report(y_test, y_pred))
print("Confusion Matrix:\n", confusion_matrix(y_test, y_pred))

# Step 8: Save Model for future use
joblib.dump(model, 'env_monitor_model.pkl')
joblib.dump(scaler, 'scaler.pkl')

# Step 9: Visualization
plt.figure(figsize=(12, 6))
sns.lineplot(x='timestamp', y='temperature', data=df, label='Temperature')
sns.lineplot(x='timestamp', y='air_quality', data=df, label='Air Quality')
plt.axhline(35, color='r', linestyle='--', label='Temp Threshold')
plt.axhline(600, color='orange', linestyle='--', label='Air Quality Threshold')
plt.legend()
plt.title("Environmental Trends Over Time")
plt.xticks(rotation=45)
plt.tight_layout()
plt.show()

# Optional: Save cleaned and labeled dataset
df.to_csv("sensor_data_cleaned.csv", index=False)
