import os
import gzip
import shutil

compressable_files = ("html", "css", "js", "ico")
indir = "./data_uncompressed"
outdir = "./data"

# When ./data exsits, delte the folder and all files in it
if os.path.exists(outdir):
  shutil.rmtree(outdir)
  print('Removed ./data')
# Create ./data if it doesn't exsit
if not os.path.exists(outdir):
  os.makedirs(outdir)
  print('Created ./data')

# Loop through all files in ./data_uncompressed
for root, dirs, files in os.walk(indir):
  for filename in files:
    compressable = filename.endswith(compressable_files)
    if compressable:
      print("Compressing:\t" + filename)
      with open(indir + "/" + filename, "rb") as f_in:
        with gzip.open(outdir + "/" + filename + ".gz", "wb") as f_out:
          shutil.copyfileobj(f_in, f_out)
    else:
      print("Copying:\t" + filename)
      shutil.copyfile(indir + "/" + filename, outdir + "/" + filename)

print("\nCopying done!")