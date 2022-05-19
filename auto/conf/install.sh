#!/bin/bash

register=https://dev.aosings.top/api/product-node-service/node/nodes/register/a36z74yzuxmggk4stis5t6wj7yzhg299

function SYS_VERSION()
{
    if grep -Eqii "CentOS" /etc/issue || grep -Eq "CentOS" /etc/*-release; then
        DISTRO='CentOS'
        PM='yum'
    elif grep -Eqi "Red Hat Enterprise Linux Server" /etc/issue || grep -Eq "Red Hat Enterprise Linux Server" /etc/*-release; then
        DISTRO='RHEL'
        PM='yum'
    elif grep -Eqi "Aliyun" /etc/issue || grep -Eq "Aliyun" /etc/*-release; then
        DISTRO='Aliyun'
        PM='yum'
    elif grep -Eqi "Fedora" /etc/issue || grep -Eq "Fedora" /etc/*-release; then
        DISTRO='Fedora'
        PM='yum'
    elif grep -Eqi "Debian" /etc/issue || grep -Eq "Debian" /etc/*-release; then
        DISTRO='Debian'
        PM='apt'
    elif grep -Eqi "Ubuntu" /etc/issue || grep -Eq "Ubuntu" /etc/*-release; then
        DISTRO='Ubuntu'
        PM='apt'
    elif grep -Eqi "Raspbian" /etc/issue || grep -Eq "Raspbian" /etc/*-release; then
        DISTRO='Raspbian'
        PM='apt'
    else
        DISTRO='unknow'
    fi
	
    echo $DISTRO
}

function CheckAndInstalled() {
	if ! rpm -qa|grep -q "^$1"
	then
		echo Checking for $1 is not found.
		echo "Ready to install."

		yum install -y $1
		CheckForInstalled
	else
		echo Checking for $1 is found.
	fi
}

function CheckForInstalled() {

	if [ $? != 0 ]
	then
		echo "Installed Error, Check the error log."
		echo "Error - Break, exit(-1)."
		exit 0
	fi
}

# Checking for Asnode
for p in pcre pcre-devel perl wget zip zlib zlib-devel curl curl-devel;
do 
	CheckAndInstalled $p;
done

function CheckForUrl() {

	checkUrl=($(curl -i -s --connect-timeout 2 $1 | head -1 | tr "\r" "\n"))

	if [ "${checkUrl[1]}" == '200' -a "${checkUrl[2]}" == 'OK' ];
	then
		echo Checking for URL is correct: $1.
	else 
		exit 0
	fi
}


if [[ `SYS_VERSION` =~ "CentOS" ]];
then
	asnodeUrl=http://godlogic.top/download/asnode.centos.tar.gz
	CheckForUrl $asnodeUrl

	echo "Downloading: $asnodeUrl"

	curl -O $asnodeUrl
	tar -zxvf asnode.centos.tar.gz
	
	if [ $? -eq 0 ];
	then
		rm -rf asnode.centos.tar.gz asnode
	fi

	if [ -d ./Asnode ]
	then
		mv Asnode asnode
	fi

elif [[ `SYS_VERSION` =~ "Ubuntu" ]];
then
	echo "Don't hava any packege/Asnode for Ubuntu."
	exit 0
else
	exit 0
fi

echo "Download is complete."

CheckForUrl $register
curl -o node.json $register

if [ $? -ne 0 ];
then
    echo "Register info is not found."
	exit 0

else
	if [ -f node.json ]
	then
		cp -rf node.json ./asnode/conf/
	else
		echo "Register info is not found."
		exit 0
	fi
fi

# copy file to ${Asnode}
prefix=/usr/local
Asnode=${prefix}"/asnode"

if [ -d ${Asnode} ]
then
	nodePID=`ps -ef | grep -w Asnode | grep -v grep | wc -l`
	
	if [ 0 -lt $nodePID ];
	then
		echo "Asnode already installed."

		nodeStop=${Asnode}"/sbin/Asnode -s stop"
		echo "Start: $nodeStop."
		$nodeStop

		nodeKill=`ps -ef | grep -w Asnode | grep -v grep | awk '{print $2}' | xargs kill -9`
		$nodeKill
	fi

	rm -rf ${Asnode}
fi

# copy file
cp -rf ./asnode ${Asnode}

sleep 1

if [ $? -ne 0 ];then
    echo "Asnode failed to run."
    echo "Error exit."
	exit 0
else
    echo "Asnode is successfully."
fi

# Checking for Asnode
for p in pcre pcre-devel perl wget zip zlib zlib-devel openssl-devel;
do 
	CheckAndInstalled $p;
done

# Copy openresty 
nginx=${prefix}"/openresty"
echo "Check for $nginx."

if [ -d ${nginx} ]
then
    echo "nginx already installed."

	# ask installer, input a correct choose
	ngxPID=`ps -ef | grep -w nginx | grep -v grep | wc -l`
	if [ 0 -lt $ngxPID ]
	then
		ngxStop=${prefix}"/openresty/nginx/sbin/nginx -s stop"
		echo "Start: $ngxStop."
		$ngxStop
		
		if [ $? -ne 0 ];then
			nodeKill=`ps -ef | grep -w nginx | grep -v grep | awk '{print $2}' | xargs kill -9`
			$nodeKill
		fi
	fi

	if [ $? -eq 0 ]; then
		rm -rf ${nginx}
	fi
	
fi


if [[ `SYS_VERSION` =~ "CentOS" ]];
then
	openrestyURL=http://godlogic.top/download/openresty.centos.tar.gz

	CheckForUrl $openrestyURL
	echo "Downloading:" $openrestyURL

	curl -O $openrestyURL
	tar -xzf openresty.centos.tar.gz

	if [ $? -eq 0 ];then
		rm -rf openresty.centos.tar.gz
		cp -rf openresty ${nginx}
	fi

elif [[ `SYS_VERSION` =~ "Ubuntu" ]]; then
	echo "Don's hava any packege/nginx for Ubuntu."
	exit 0
else
	echo "Unknow what is system/linux!"
	exit 0
fi

if [ $? -ne 0 ];then
    echo "Openresty installed is failed."
    echo "Error exit."
	exit 0
else
    echo "Openresty installed is successfully."
fi

# create /data/nginx/*
mkdir -p /data/nginx/cache/

# nginx.sock
rm -rf /var/run/nginx
mkdir -p /var/run/nginx/

# openresty/logs
mkdir -p /usr/local/openresty/nginx/conf/domain/
mkdir -p /usr/local/openresty/nginx/conf/stream/
mkdir -p /usr/local/openresty/nginx/conf/lua/
mkdir -p /usr/local/openresty/nginx/logs/domain/
mkdir -p /usr/local/openresty/nginx/logs/stream/

# start ngin
nginxBin=${nginx}"/nginx/sbin/nginx"
echo "Start: $nginxBin."
$nginxBin

# start asnode
asnodeBin=${Asnode}"/sbin/Asnode -s reload"
asnodePID=`ps -ef | grep Asnode | grep -v grep | wc -l`
if [ 0 -eq $asnodePID ]
then
	asnodeBin=${Asnode}"/sbin/Asnode"
fi

echo "Start: $asnodeBin."
$asnodeBin

#
findNodes=`cat '/etc/rc.local' | grep '/usr/local/asnode/start.sh' | wc -L`

#
if [ 0 -eq $findNodes ];
then
        chmod +x '/etc/rc.local'
        echo '/usr/local/asnode/start.sh' >> '/etc/rc.local'
fi
#done 
