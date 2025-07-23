import time
import smtplib
from email.mime.text import MIMEText
from firebase_admin import credentials, db, initialize_app

# Initialize Firebase
cred = credentials.Certificate("firebaseSDK.json")  # tải từ Firebase Service Account
initialize_app(cred, {
    'databaseURL': 'firebase_URL'
})

# Email settings
EMAIL_SENDER = "sender email address"
EMAIL_PASSWORD = "app_password"
EMAIL_RECEIVER = "receiver email address"

def get_sensor_data():
    return db.reference("sensors").get()

def send_email(data):
    subject = "Fire Alarm Notification"
    body = f"""
    <b>Fire Alarm Triggered</b> 

    <ul>
        <li><b>Temperature:</b> {data.get('temperature', '--')} °C</li>
        <li><b>Humidity:</b> {data.get('humidity', '--')} %</li>
        <li><b>Smoke:</b> {data.get('gas_smoke', '--')}</li>
        <li><b>Alarm:</b> {'ON' if data.get('alarm') else 'OFF'}</li>
    </ul>
    """

    msg = MIMEText(body, 'html')
    msg['Subject'] = subject
    msg['From'] = EMAIL_SENDER
    msg['To'] = EMAIL_RECEIVER

    try:
        with smtplib.SMTP_SSL('smtp.gmail.com', 465) as smtp:
            smtp.login(EMAIL_SENDER, EMAIL_PASSWORD)
            smtp.send_message(msg)
        print("Email sent.")
    except Exception as e:
        print("Email sending failed:", e)

def run_notifier():
    while True:
        data = get_sensor_data()
        if not data:
            print("No sensor data found.")
        elif data.get("alarm", False):
            send_email(data)
        else:
            print("Alarm is OFF. No email sent.")

        time.sleep(3)

if __name__ == "__main__":
    run_notifier()
