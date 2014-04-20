######################################################################
# Assistant
######################################################################
rc assistant.rdef
xres -o "../bin/assistant" assistant.rsrc
mimeset -f "../bin/assistant"

######################################################################
# Designer
######################################################################
rc designer.rdef
xres -o "../bin/designer" designer.rsrc
mimeset -f "../bin/designer"

######################################################################
# Linguist
######################################################################
rc linguist.rdef
xres -o "../bin/linguist" linguist.rsrc
mimeset -f "../bin/linguist"

######################################################################
# Help Conversion Wizard
######################################################################
rc qhelpconverter.rdef
xres -o "../bin/qhelpconverter" qhelpconverter.rsrc
mimeset -f "../bin/qhelpconverter"

######################################################################
# QML Viewer
######################################################################
rc qmlviewer.rdef
xres -o "../bin/qmlviewer" qmlviewer.rsrc
mimeset -f "../bin/qmlviewer"

######################################################################
# Config
######################################################################
rc qtconfig.rdef
xres -o "../bin/qtconfig" qtconfig.rsrc
mimeset -f "../bin/qtconfig"
