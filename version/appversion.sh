#!/bin/bash

#
# Ported from s1lentq/reapi
#

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
	
	#
	# Configure remote url repository
	#
	# Get remote name by current branch
	BRANCH_REMOTE=$(git -C "$GIT_DIR" config branch.$BRANCH_NAME.remote)
	if [ $? -ne 0 -o "$BRANCH_REMOTE" = "" ]; then
		BRANCH_REMOTE=origin
	fi

	# Get commit id
	COMMIT_SHA=$(git -C "$GIT_DIR" rev-parse --verify HEAD)
	COMMIT_SHA=${COMMIT_SHA:0:7}

	# Get remote url
	COMMIT_URL=$(git -C "$GIT_DIR" config remote.$BRANCH_REMOTE.url)

	URL_CONSTRUCT=0

	if [[ "$COMMIT_URL" == *"git@"* ]]; then

		URL_CONSTRUCT=1

		# Strip prefix 'git@'
		COMMIT_URL=${COMMIT_URL#git@}

		# Strip postfix '.git'
		COMMIT_URL=${COMMIT_URL%.git}

		# Replace ':' to '/'
		COMMIT_URL=${COMMIT_URL/:/\/}

	elif [[ "$COMMIT_URL" == *"https://"* ]]; then

		URL_CONSTRUCT=1

		# Strip prefix 'https://'
		COMMIT_URL=${COMMIT_URL#https://}

		# Strip postfix '.git'
		COMMIT_URL=${COMMIT_URL%.git}

	fi

	if test "$URL_CONSTRUCT" -eq 1; then
		COMMIT_URL=$(echo https://$COMMIT_URL/commit/)
	fi
	
	NEW_VERSION_INC="$MAJOR$MINOR$MAINTENANCE$COMMIT_COUNT"
	NEW_VERSION="$MAJOR.$MINOR.$MAINTENANCE.$COMMIT_COUNT"
	
	echo "NEW_VERSION_INC=${NEW_VERSION_INC}" >> $GITHUB_ENV
	echo "NEW_VERSION=${NEW_VERSION}" >> $GITHUB_ENV
	
	update_appversion
}

update_appversion()
{
	day=$(date +%d)
	year=$(date +%Y)
	hours=$(date +%H:%M:%S)
	month=$(LANG=en_us_88591; date +"%b")

	APPVERSION_CONTENT=$(echo -e "\n")
	APPVERSION_CONTENT+=$(echo -e "//\n")
	APPVERSION_CONTENT+=$(echo -e "// This file is generated automatically.\n")
	APPVERSION_CONTENT+=$(echo -e "// Don't edit it.\n")
	APPVERSION_CONTENT+=$(echo -e "//\n")
	APPVERSION_CONTENT+=$(echo -e "\n")
	APPVERSION_CONTENT+=$(echo -e "// MultiMod Manager version\n")
	APPVERSION_CONTENT+=$(echo -e "#define MM_VERSION $NEW_VERSION_INC\n")
	APPVERSION_CONTENT+=$(echo -e "#define MM_VERSION_MAJOR $MAJOR\n")
	APPVERSION_CONTENT+=$(echo -e "#define MM_VERSION_MINOR $MINOR\n")
	APPVERSION_CONTENT+=$(echo -e "#define MM_VERSION_MAINTENANCE $MAINTENANCE\n")
	APPVERSION_CONTENT+=$(echo -e "#define MM_VERSION_COMMIT $COMMIT_COUNT\n")
	APPVERSION_CONTENT+=$(echo -e '#define MM_VERSION_STRD "'$NEW_VERSION'"\n')
	APPVERSION_CONTENT+=$(echo -e "\n")
	APPVERSION_CONTENT+=$(echo -e '#define MM_COMMIT_DATE "'$month $day $year'"\n')	
	APPVERSION_CONTENT+=$(echo -e '#define MM_COMMIT_TIME "'$hours'"\n')
	APPVERSION_CONTENT+=$(echo -e "\n")
	APPVERSION_CONTENT+=$(echo -e '#define MM_COMMIT_SHA "'$COMMIT_SHA'"\n')
	APPVERSION_CONTENT+=$(echo -e '#define MM_COMMIT_URL "'$COMMIT_URL'"\n')
	APPVERSION_CONTENT+=$(echo -e "\n")

	echo Updating $APPVERSION_FILE, new version is '"'$NEW_VERSION'"'
	
	echo -e "#if defined _mm_version_included_\r">$APPVERSION_FILE
	echo -e "	#endinput\r">>$APPVERSION_FILE
	echo -e "#endif\r">>$APPVERSION_FILE
	echo -e "#define _mm_version_included_\r">>$APPVERSION_FILE
	echo -e $APPVERSION_CONTENT>>$APPVERSION_FILE
	echo -e '#define PLUGIN_VERSION "'$NEW_VERSION'"\r'>>$APPVERSION_FILE

	cat $APPVERSION_FILE
	
	echo Updating $APPVERSION_FILE_NATIVES, new version is '"'$NEW_VERSION'"'
	
	echo -e "#if defined _multimod_manager_version_included_\r">$APPVERSION_FILE_NATIVES
	echo -e "	#endinput\r">>$APPVERSION_FILE_NATIVES
	echo -e "#endif\r">>$APPVERSION_FILE_NATIVES
	echo -e "#define _multimod_manager_version_included_\r">>$APPVERSION_FILE_NATIVES
	echo -e $APPVERSION_CONTENT>>$APPVERSION_FILE_NATIVES
	echo -e '#define MM_NATIVES_API_VER "'$NEW_VERSION'"\r'>>$APPVERSION_FILE_NATIVES

	cat $APPVERSION_FILE_NATIVES
}

# Initialise
init $*

# Exit normally
exit 0