#!/bin/bash

OUTPUT="result-src.txt"

> $OUTPUT

echo "Combining ..."

for f in *.cpp; do
    echo "Processing: $f"
    cat "$f" >> $OUTPUT
    
    echo -e "\n--- End of $f ---\n" >> $OUTPUT
done

echo "Done! All files stored to $OUTPUT"
