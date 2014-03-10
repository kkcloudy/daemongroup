# 
# This file intends to git commit the changing of buildno 

BUILDNO_FILE=../aw2.0.18sp8_pb/buildno_v2.0
BUILDNO_FILE_L=./accapi/buildno_v2.0

updatebuildno:
	@echo $$((`cat ${BUILDNO_FILE}`)) > ${BUILDNO_FILE_L}
	@if [ -d .git ] ; then \
		git pull ; \
		git add ${BUILDNO_FILE_L} ; \
		git commit -m "Increased buildno to `cat ${BUILDNO_FILE_L}`" ; \
		git push ; \
	else \
		cvs commit -m "Build `cat ${BUILDNO_FILE_L}` was performed." ${BUILDNO_FILE_L} ; \
	fi
