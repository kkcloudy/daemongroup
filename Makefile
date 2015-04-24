# 
# This file intends to git commit the changing of buildno 

BUILDNO_FILE=../aw2.0.18sp8_pb/buildno_v2.0
BUILDNO_FILE_L=./accapi/buildno_v2.0
BUILDNO_FILE_22=../aw2.0.22_pb/buildno_v2.0
BUILDNO_FILE_L_22=./accapi/buildno_v2.0.22

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

updatebuildno_22:
	@echo $$((`cat ${BUILDNO_FILE_22}`)) > ${BUILDNO_FILE_L_22}
	@if [ -d .git ] ; then \
		git pull ; \
		git add ${BUILDNO_FILE_L_22} ; \
		git commit -m "Increased buildno to `cat ${BUILDNO_FILE_L_22}`" ; \
		git push ; \
	else \
		cvs commit -m "Build `cat ${BUILDNO_FILE_L_22}` was performed." ${BUILDNO_FILE_L_22} ; \
	fi
