from flask import Flask
from firebase_admin import credentials, db, initialize_app

app = Flask(__name__)

# Firebase setup (same as above)
cred = credentials.Certificate("firsbaseSDK.json")
initialize_app(cred, {
    'databaseURL': 'firebase_URL'
})

@app.route("/confirm")
def confirm():
    db.reference("sensors/confirmed").set(True)
    return "<h2> Alert confirmed. Emails will stop. You may now close this page.</h2>"

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
