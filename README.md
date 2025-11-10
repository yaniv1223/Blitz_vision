# ⚡ BLITZ VISION: AI-Powered Voice Assistant for the Visually Impaired

### A portable accessibility device for the visually impaired. It uses an ESP32 camera to capture images and an API server (GPT-4o + Google TTS) to provide immediate, descriptive audio feedback on objects.

BLITZ VISION is an innovative, portable solution that bridges a compact ESP32 camera module with powerful, cloud-based Artificial Intelligence (AI) to enhance independence and environmental awareness for the blind and visually impaired.

---

## 📹 Demo Video (הדגמה)

**[כאן מומלץ להטמיע את הקישור לסרטון ההדגמה ביוטיוב/GIF]**

[![Watch the Demo on YouTube](http://img.youtube.com/vi/[YOUR_VIDEO_ID]/0.jpg)](http://www.youtube.com/watch?v=[YOUR_VIDEO_ID])

---

## 💡 The Value Proposition (הצעת הערך)

BLITZ VISION addresses a critical need for **immediate and context-aware assistance** for the visually impaired. Unlike static identifiers, this system provides a sophisticated, natural language description of objects, enhancing the user's independence and decision-making capabilities in complex environments (such as shopping aisles or identifying household items).

The integration of the low-cost, portable **ESP32** with the high-level language understanding of **GPT-4o** and the localization of **Google TTS** creates a powerful tool that moves beyond simple object recognition to offer true **situational awareness** through clear, Hebrew audio feedback.

---

## 🔄 Data Flow Diagram: The BLITZ VISION Process

┌─────────────────────┐
│ 1. ESP32 (Edge)     │
│  - Captures image   │
│  - Sends raw bytes  │
└─────────┬───────────┘
          │
          │ (HTTP POST Request: Image Bytes)
          ▼
┌─────────────────────┐
│ 2. Flask Server     │
│  - Receives image   │
│  - Forwards to AI   │
└─────────┬───────────┘
          │
          │ (API Request: Image + Prompt)
          ▼
┌─────────────────────┐
│ 3. OpenAI / GPT-4o  │
│  - Analyzes image   │
│  - Generates Text   │
└─────────┬───────────┘
          │
          │ (API Response: Description Text)
          ▼
┌─────────────────────┐
│ 4. Flask Server     │
│  - Receives Text    │
│  - Forwards to TTS  │
└─────────┬───────────┘
          │
          │ (API Request: Description Text)
          ▼
┌─────────────────────┐
│ 5. Google Cloud TTS │
│  - Converts text to │
│    Hebrew MP3 file  │
└─────────┬───────────┘
          │
          │ (HTTP/API Response: MP3 Audio Stream)
          ▼
┌─────────────────────┐
│ 6. ESP32 (Edge)     │
│  - Receives MP3     │
│  - **Saves Locally**│
│  - **Plays Audio** │
└─────────────────────┘
