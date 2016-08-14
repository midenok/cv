#!/bin/sh
sed -i -re '
    /usepackage\[russian\]\{babel\}/ {
        i\
\\usepackage[russian=nohyphenation]{hyphsubst}
    }
' "$1"
