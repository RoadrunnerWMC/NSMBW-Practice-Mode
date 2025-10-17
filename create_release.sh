# Check for zopfli
if command -v advzip > /dev/null 2>&1; then
    HAS_ZOPFLI=1
    ZIP_OPT=-0
else
    HAS_ZOPFLI=
    ZIP_OPT=
fi

# Delete previous release zip if it exists
rm -f NEW_RELEASE.zip

# Make "TEMP" copy of "sd" folder
mkdir TEMP
cp -ra sd/* TEMP

# Remove the Riivolution config folder if it exists
# (Dolphin sometimes creates this automatically)
rm -rf TEMP/riivolution/config

# Clear the Code folder, and put the right files back in
rm TEMP/nsmbw_practice_mode/Code/*
cp -a sd/nsmbw_practice_mode/Code/loader.bin TEMP/nsmbw_practice_mode/Code/
cp -a code/bin/*.bin TEMP/nsmbw_practice_mode/Code/

# Create zip file
cd TEMP
zip $ZIP_OPT -r ../NEW_RELEASE.zip *
cd ..

# Delete TEMP folder
rm -rf TEMP

# Optional: compress with zopfli
if [ -n "$HAS_ZOPFLI" ]; then
    echo "compressing with zopfli, please wait..."
    advzip -z4 NEW_RELEASE.zip
else
    echo "advzip not found, skipping zopfli compression"
fi
