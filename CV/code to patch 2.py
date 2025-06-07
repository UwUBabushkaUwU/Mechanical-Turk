from pathlib import Path
import shutil
import io
import sys
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

folder = Path(r"C:\Users\Chinar Mhatre\Documents\Mechanical Turk\CV\chess-cv\data\raw\games\game19\rev")
backup_folder = folder.parent / (folder.name + "_backup")
backup_folder.mkdir(exist_ok=True)

# Copy original files to backup
for file in folder.glob("*.*"):
    shutil.copy(file, backup_folder / file.name)

# Sanitize and rename files
image_files = sorted(folder.glob("*.*"))
for i, file in enumerate(image_files, 1):
    ext = file.suffix.lower()
    clean_name = f"{i}{ext}"
    clean_path = folder / clean_name
    if clean_path != file:
        print(f"Renaming {repr(file.name)} → {clean_name}")
        file.rename(clean_path)
