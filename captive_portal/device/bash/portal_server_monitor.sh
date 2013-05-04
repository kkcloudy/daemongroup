#/bin/bash
#”√¿¥º‡ ”portal server «∑Òø…”√£¨»Áπ˚≤ªø…”√£¨æÕ∑¢ÀÕtrap°
#’‚∏ˆΩ≈±æ «“ª∏ˆ ÿª§Ω¯≥Ã£¨√ø∏Ù“ª∂Œ ±º‰÷Æ∫Û£¨æÕª·»•pingœ‡”¶µƒportal server ip£¨÷ªping“ª¥Œ ping√¸¡Ó“≤ «
#œ¬“ª¥Œµƒ ±∫Úª·œ»ºÏ≤È÷Æ«∞ping√¸¡ÓµƒΩ·π˚°£»Áπ˚¡¨–¯»˝¥Œping√¸¡Ó∂º≤ªƒ‹Õ®£¨æÕ∑¢ÀÕtrap°£
#’‚∏ˆΩ≈±æ¥Û≤ø∑÷µƒ ±º‰∂º «‘⁄sleep£¨≤ªª·’º”√cpuÃ´∂‡ ±º‰°£

source cp_start.sh

PORTAL_UNREACHABLE_IPS=""

if [ ! -f ${CP_DB_FILE} ];then
	exit
fi

while read line
do
	id=`echo $line | awk '{print $1}'`
	ip=`echo $line | awk '{print $2}'`

	ping.sh $ip >/dev/null 2>&1
	if [ ! $? == "0" ];then
		#∏√portalœ‡πÿµƒ–≈œ¢
		if [ "${PORTAL_UNREACHABLE_IPS}" == "" ];then

			PORTAL_UNREACHABLE_IPS=${ip}
		else

			PORTAL_UNREACHABLE_IPS="${PORTAL_UNREACHABLE_IPS},${ip}"
		fi
	fi
done < ${CP_DB_FILE}

if [ ! "${PORTAL_UNREACHABLE_IPS}" == "" ];then
	printf ${PORTAL_UNREACHABLE_IPS}
fi
