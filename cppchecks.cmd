cppcheck --enable=all --platform=win64 --suppress=missingIncludeSystem --force --report-progress --quiet -iexternals/ -I%cd%/engine/src/ engine/src