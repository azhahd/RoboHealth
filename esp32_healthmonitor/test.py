from google.cloud import vision
import os

# Check if the environment variable is set
print("GOOGLE_APPLICATION_CREDENTIALS:", os.getenv("GOOGLE_APPLICATION_CREDENTIALS"))

# Try initializing Google Vision API client
try:
    client = vision.ImageAnnotatorClient()
    print("Google Vision API Authentication Successful!")
except Exception as e:
    print("Error:", e)
