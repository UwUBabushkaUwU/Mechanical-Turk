import cv2
from PIL import Image
import numpy as np
from pathlib import Path
import os
import sys


def reencode_with_cv_fallback(folder):
    folder = Path(folder)
    for ext in ('*.jpg', '*.jpeg'):
        for img_path in folder.rglob(ext):
            try:
                # Convert path to string in UTF-8 encoding
                path_str = str(img_path)

                print("Trying:", repr(img_path.name))


                img_cv = cv2.imread(path_str)
                if img_cv is None:
                    print(f"⚠️ OpenCV failed to read {path_str}")
                    continue

                img_rgb = cv2.cvtColor(img_cv, cv2.COLOR_BGR2RGB)
                pil_img = Image.fromarray(img_rgb)

                # Temp path should also be clean
                temp_path = img_path.with_name(img_path.stem + ".temp.jpg")

                pil_img.save(str(temp_path), format='JPEG', quality=95)
                os.replace(str(temp_path), path_str)

                print("Re-encoded:", img_path.name.encode('ascii', 'ignore').decode())
            except Exception as e:
                print(" Failed:", img_path.name.encode('ascii', 'ignore').decode(), "|", str(e))


# Set your base image folder here
base_folder = r"C:\Users\Chinar Mhatre\Documents\Mechanical Turk\CV\chess-cv\data\raw\games\game19\rev"

reencode_with_cv_fallback(base_folder)
