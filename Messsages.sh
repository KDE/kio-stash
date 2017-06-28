#!/bin/sh

# invoke the extractrc script on all .ui, .rc, and .kcfg files in the sources
# the results are stored in a pseudo .cpp file to be picked up by xgettext.
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
# invoke the grantlee extract script for translatable string from Grantlee themes
$EXTRACT_GRANTLEE_TEMPLATE_STRINGS `find . -name \*.html` >> html.cpp
# call xgettext on all source files. If your sources have other filename
# extensions besides .cc, .cpp, and .h, just add them in the find call.
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h -name \*.qml` -o $podir/APPNAME.pot
