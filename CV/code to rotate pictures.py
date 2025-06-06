from PIL import Image
import os

# Folder path with images
folder_path = r'C:\Users\Chinar Mhatre\Downloads\Game-19\Black'

# Supported image extensions
extensions = ('.jpg', '.jpeg', '.png', '.bmp', '.tiff', '.gif')

for filename in os.listdir(folder_path):
    if filename.lower().endswith(extensions):
        image_path = os.path.join(folder_path, filename)
        try:
            with Image.open(image_path) as img:
                width, height = img.size
                
                if width <= height:
                    rotated_img = img.rotate(90, expand=True)
                    rotated_img.save(image_path)
                    print(f"Rotated {filename} to landscape mode.")
                else:
                    print(f"{filename} is already in landscape mode.")
        except Exception as e:
            print(f"Error processing {filename}: {e}")