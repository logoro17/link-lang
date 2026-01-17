#!/bin/bash

OUTPUT="result-include.txt"

> $OUTPUT

echo "Merging files ..."

for f in *.h; do
    echo "Processing: $f"

    cat "$f" >> $OUTPUT

    echo -e "\n--- End of $f ---\n" >> $OUTPUT
done

echo "Done! All Files Mixed to $OUTPUT"
