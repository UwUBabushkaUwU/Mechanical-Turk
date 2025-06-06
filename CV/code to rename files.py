import os
import shutil

# Define the source directory
source_dir = r"C:\Users\Chinar Mhatre\Downloads\Game-3\White"
# Define the destination directory for renamed copies
destination_dir = os.path.join(source_dir, "RenamedCopies")

# Create the destination directory if it doesn't exist
os.makedirs(destination_dir, exist_ok=True)

# Get all .jpg files in the directory, sorted by name
jpg_files = sorted(f for f in os.listdir(source_dir) if f.lower().endswith(".jpeg"))

# Copy and rename files
for idx, filename in enumerate(jpg_files, start=1):
    src_path = os.path.join(source_dir, filename)
    dst_path = os.path.join(destination_dir, f"{idx}.jpg")
    shutil.copy2(src_path, dst_path)

print(f"Copied and renamed {len(jpg_files)} files to '{destination_dir}'.")
