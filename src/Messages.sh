#!/bin/sh

$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$EXTRACT_GRANTLEE_TEMPLATE_STRINGS `find . -name \*.html` >> html.cpp
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h -name \*.qml` -o $podir/kio-stash.pot
