from flask import Flask, request, send_file, jsonify
from openai import OpenAI
import base64, os, tempfile
import requests
from dotenv import load_dotenv

load_dotenv()
OPENAI_API_KEY = os.getenv("OPENAI_API_KEY")
GOOGLE_TTS_API_KEY = os.getenv("GOOGLE_TTS_API_KEY")
PROMPT = "תאר את האוביקט ללא מסביב ,תשמש מקסימום ב20 מילים"

app = Flask(__name__)

def get_image_description(base64_image, prompt):
    client = OpenAI(api_key=OPENAI_API_KEY)
    chat_completion = client.chat.completions.create(
        model="gpt-4o",
        max_tokens=50,
        messages=[{
            "role": "user",
            "content": [
                {"type": "text", "text": prompt},
                {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{base64_image}"}}
            ]
        }]
    )
    return chat_completion.choices[0].message.content

def google_tts(text, api_key, output_filename):
    url = f"https://texttospeech.googleapis.com/v1/text:synthesize?key={api_key}"
    headers = {"Content-Type": "application/json"}
    data = {
        "input": {"text": text},
        "voice": {"languageCode": "he-IL", "name": "he-IL-Standard-A"},
        "audioConfig": {"audioEncoding": "MP3"}
    }
    response = requests.post(url, headers=headers, json=data)
    response.raise_for_status()
    audio_content = response.json()["audioContent"]
    with open(output_filename, "wb") as out:
        out.write(base64.b64decode(audio_content))

@app.route("/upload", methods=["POST"])
def upload():
    image_bytes = request.data
    if not image_bytes:
        return jsonify({"error": "No image uploaded"}), 400

    base64_image = base64.b64encode(image_bytes).decode('utf-8')
    description = get_image_description(base64_image, PROMPT)

    with tempfile.NamedTemporaryFile(delete=False, suffix=".mp3") as tmp:
        google_tts(description, GOOGLE_TTS_API_KEY, tmp.name)
        return send_file(tmp.name, mimetype='audio/mpeg', as_attachment=False)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)