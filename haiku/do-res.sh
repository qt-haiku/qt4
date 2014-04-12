######################################################################
# Assistant
######################################################################
rc assistant.rdef
xres -o "$QTROOT/boot/common/bin/assistant" assistant.rsrc
mimeset -f "$QTROOT/boot/common/bin/assistant"

######################################################################
# Designer
######################################################################
rc designer.rdef
xres -o "$QTROOT/boot/common/bin/designer" designer.rsrc
mimeset -f "$QTROOT/boot/common/bin/designer"

######################################################################
# Linguist
######################################################################
rc linguist.rdef
xres -o "$QTROOT/boot/common/bin/linguist" linguist.rsrc
mimeset -f "$QTROOT/boot/common/bin/linguist"

######################################################################
# Help Conversion Wizard
######################################################################
rc qhelpconverter.rdef
xres -o "$QTROOT/boot/common/bin/qhelpconverter" qhelpconverter.rsrc
mimeset -f "$QTROOT/boot/common/bin/qhelpconverter"

######################################################################
# QML Viewer
######################################################################
rc qmlviewer.rdef
xres -o "$QTROOT/boot/common/bin/qmlviewer" qmlviewer.rsrc
mimeset -f "$QTROOT/boot/common/bin/qmlviewer"

######################################################################
# Config
######################################################################
rc qtconfig.rdef
xres -o "$QTROOT/boot/common/bin/qtconfig" qtconfig.rsrc
mimeset -f "$QTROOT/boot/common/bin/qtconfig"
