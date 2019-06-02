import os
import gzip
import shutil

compressable_files = ("html", "css", "js", "ico", "svg", "map", "json", "txt")
indir = "./webfiles/dist"
outdir = "./data"

# When outdir exsits, delte the folder and all files in it
if os.path.exists(outdir):
  shutil.rmtree(outdir)
  print("Removed " + outdir)
# Create outdir if it doesn't exsit
if not os.path.exists(outdir):
  os.makedirs(outdir)
  print("Created " + outdir)

# Loop through all directories in "indir" and create folder tree
for dirpath, dirnames, files in os.walk(indir):
  dirname = outdir + dirpath.replace(indir, "")
  if not os.path.exists(dirname):
    os.makedirs(dirname)
    print("Created " + dirname)

# Loop through all files in "indir"
for dirpath, dirnames, files in os.walk(indir):
  for filename in files:
    filename = os.path.join(dirpath, filename).replace(indir, "")[1:].replace("\\", "/")
    compressable = filename.endswith(compressable_files)
    if compressable:
      print("Compressing:\t" + filename)
      with open(indir + "/" + filename, "rb") as f_in:
        with gzip.open(outdir + "/" + filename + ".gz", "wb") as f_out:
          shutil.copyfileobj(f_in, f_out)
    else:
      print("Copying:\t" + filename)
      shutil.copyfile(indir + "/" + filename, outdir + "/" + filename)

print("\nCopying & compressing done!")