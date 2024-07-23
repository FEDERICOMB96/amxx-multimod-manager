#!/bin/bash

init()
{
	SOURCE_DIR=$1
	GIT_DIR=$SOURCE_DIR
	VERSION_FILE=$SOURCE_DIR/version/version.h
	APPVERSION_FILE=$SOURCE_DIR/version/version.inc
	APPVERSION_FILE_NATIVES=$SOURCE_DIR/version/multimod_manager_version.inc
	
	MAJOR=$(cat "$VERSION_FILE" | grep -wi 'MM_VERSION_MAJOR' | sed -e 's/.*MM_VERSION_MAJOR.*[^0-9]\([0-9][0-9]*\).*/\1/i' -e 's/\r//g')
	if [ $? -ne 0 -o "$MAJOR" = "" ]; then
		MAJOR=0
	fi
	
	MINOR=$(cat "$VERSION_FILE" | grep -wi 'MM_VERSION_MINOR' | sed -e 's/.*MM_VERSION_MINOR.*[^0-9]\([0-9][0-9]*\).*/\1/i' -e 's/\r//g')
	if [ $? -ne 0 -o "$MINOR" = "" ]; then
		MINOR=0
	fi
	
	MAINTENANCE=$(cat "$VERSION_FILE" | grep -i 'MM_VERSION_MAINTENANCE' | sed -e 's/.*MM_VERSION_MAINTENANCE.*[^0-9]\([0-9][0-9]*\).*/\1/i' -e 's/\r//g')
	if [ $? -ne 0 -o "$MAINTENANCE" = "" ]; then
		MAINTENANCE=0
	fi

	BRANCH_NAME=$(git -C "$GIT_DIR" rev-parse --abbrev-ref HEAD)
	if [ $? -ne 0 -o "$BRANCH_NAME" = "" ]; then
		BRANCH_NAME=master
	fi
	
	COMMIT_COUNT=$(git -C "$GIT_DIR" rev-list --count $BRANCH_NAME)
	if [ $? -ne 0 -o "$COMMIT_COUNT" = "" ]; then
		COMMIT_COUNT=0
	fi
	
	NEW_VERSION_INC="$MAJOR$MINOR$MAINTENANCE$COMMIT_COUNT"
	NEW_VERSION="$MAJOR.$MINOR.$MAINTENANCE.$COMMIT_COUNT"
	
	echo "NEW_VERSION_INC=${NEW_VERSION_INC}" >> $GITHUB_ENV
	echo "NEW_VERSION=${NEW_VERSION}" >> $GITHUB_ENV
	
	update_appversion
}

update_appversion()
{
	echo Updating $APPVERSION_FILE, new version is '"'$NEW_VERSION'"'
	
	echo -e "#if defined _mm_version_included_\r">$APPVERSION_FILE
	echo -e "	#endinput\r">>$APPVERSION_FILE
	echo -e "#endif\r">>$APPVERSION_FILE
	echo -e "#define _mm_version_included_\r">>$APPVERSION_FILE
	echo -e "\r">>$APPVERSION_FILE
	echo -e "// MultiMod Manager version\r">>$APPVERSION_FILE
	echo -e "#define MM_VERSION $NEW_VERSION_INC\r">>$APPVERSION_FILE
	echo -e "#define MM_VERSION_MAJOR $MAJOR\r">>$APPVERSION_FILE
	echo -e "#define MM_VERSION_MINOR $MINOR\r">>$APPVERSION_FILE
	echo -e "#define MM_VERSION_MAINTENANCE $MAINTENANCE\r">>$APPVERSION_FILE
	echo -e "#define MM_VERSION_COMMIT $COMMIT_COUNT\r">>$APPVERSION_FILE
	echo -e "\r">>$APPVERSION_FILE
	echo -e "#define PLUGIN_VERSION fmt("v%d.%d.%d.%d", MM_VERSION_MAJOR, MM_VERSION_MINOR, MM_VERSION_MAINTENANCE, MM_VERSION_COMMIT)\r">>$APPVERSION_FILE
	
	echo Updating $APPVERSION_FILE_NATIVES, new version is '"'$NEW_VERSION'"'
	
	echo -e "#if defined _multimod_manager_version_included_\r">$APPVERSION_FILE_NATIVES
	echo -e "	#endinput\r">>$APPVERSION_FILE_NATIVES
	echo -e "#endif\r">>$APPVERSION_FILE_NATIVES
	echo -e "#define _multimod_manager_version_included_\r">>$APPVERSION_FILE_NATIVES
	echo -e "\r">>$APPVERSION_FILE_NATIVES
	echo -e "// MultiMod Manager version\r">>$APPVERSION_FILE_NATIVES
	echo -e "#define MM_VERSION $NEW_VERSION_INC\r">>$APPVERSION_FILE_NATIVES
	echo -e "#define MM_VERSION_MAJOR $MAJOR\r">>$APPVERSION_FILE_NATIVES
	echo -e "#define MM_VERSION_MINOR $MINOR\r">>$APPVERSION_FILE_NATIVES
	echo -e "#define MM_VERSION_MAINTENANCE $MAINTENANCE\r">>$APPVERSION_FILE_NATIVES
	echo -e "#define MM_VERSION_COMMIT $COMMIT_COUNT\r">>$APPVERSION_FILE_NATIVES
	echo -e "\r">>$APPVERSION_FILE_NATIVES
	echo -e "#define MM_NATIVES_API_VER fmt("v%d.%d.%d.%d", MM_VERSION_MAJOR, MM_VERSION_MINOR, MM_VERSION_MAINTENANCE, MM_VERSION_COMMIT)\r">>$APPVERSION_FILE_NATIVES
}

# Initialise
init $*

# Exit normally
exit 0