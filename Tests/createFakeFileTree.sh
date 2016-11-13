#!/bin/sh

destDir="fakeFileTree"

find W:/Videos -type f | cut -c11- | while read line;
do
    echo "treating $line"
    dir=$(dirname "$line");
    if [ ! -d "$dir" ]; then
        echo "create directory $dir"
        mkdir -p "$destDir/$dir";
    else
        echo "already exists: $dir"
    fi
    touch "$destDir/$line";
done
