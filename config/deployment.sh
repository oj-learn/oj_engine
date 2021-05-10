#! /bin/bash

# ==========================================================================
# 环境变量初始化
# ==========================================================================
dates=`date +%Y%m%d`
cur_path=`pwd`

appid="$1"
dir_local="$2"
sshname="$3"
path_remote="$4"
file_supervisor="$5"

pack_name="app_${appid}_${dates}"
dir_pack="./${pack_name}"
zip_name="${pack_name}.zip"
path_zip="${cur_path}/../${zip_name}"
path_remote_supervisor_conf="${path_remote}/config/${file_supervisor}"
path_remote_zip="${path_remote}/${zip_name}"

 pattern='(program:)[^\]]*'
 supervisorctl_pragram=`grep -Po "$pattern" ${dir_local}/config/${file_supervisor}`
 supervisorctl_pragram=${supervisorctl_pragram/program:/}
 if [ -z "${supervisorctl_pragram}" ]; then
   echo "${file_supervisor} pattern '(program:)[^\]]*' null"
   exit 1
 fi

 pattern='(group:)[^\]]*'
 supervisorctl_group=`grep -Po "$pattern" ${dir_local}/config/${file_supervisor}`
 supervisorctl_group=${supervisorctl_group/group:/}
 if [ -n "${supervisorctl_group}" ]; then
  supervisorctl_pragram="${supervisorctl_group}:${supervisorctl_pragram}"
 fi  

echo "---------------------------------"
echo "pwd:`pwd`"
echo "1-appid:${appid}"
echo "2-dir_local:${dir_local}"
echo "3-sshname:${sshname}"
echo "4-path_remote:${path_remote}"
echo "5-file_supervisor:${file_supervisor}"
echo "---------------------------------"
echo "pack_name:${pack_name}"
echo "dir_pack:${dir_pack}"
echo "zip_name:${zip_name}"
echo "path_zip:${path_zip}"
echo "path_remote_zip:${path_remote_zip}"
echo "path_remote_supervisor_conf:${path_remote_supervisor_conf}"
echo "supervisorctl_pragram:${supervisorctl_pragram}"
echo "---------------------------------"

# ---------------------------------------------------------------------------------
# branch_pack
# ---------------------------------------------------------------------------------
function pack_(){

 echo "---------------------------------pack_ start"
 # 
 if [ ! -d "${dir_local}" ]; then
     echo "dir_local not: $dir_local"
     exit 1
 fi

 # delete old 
 if [ -d "${dir_pack}" ]; then
  rm -r "${dir_pack}"
 fi

 #
 mkdir -p ${dir_pack}
 #echo "cp: ${dir_local}/* ${dir_pack}/"
 cp -r ${dir_local}/* ${dir_pack}/
 cp ./demo ${dir_pack}/

 #
 cd ${dir_pack}
 zip -rq ${path_zip} ./*
 cd ..
 rm -rf ${dir_pack}
 rm -rf ${dir_local}

 #
 echo "---------------------------------pack_ end"
}

# ---------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------
function uploader_(){

 #
 echo "---------------------------------uploader_ start"

 #
 if [ ! -f "${path_zip}"  ]; then
    echo "pack not: ${path_zip}"
    exit 1
 fi

 #
 ssh "${sshname}" "[ ! -d ${path_remote} ] && mkdir -p ${path_remote}" 
 scp ${path_zip} ${sshname}:${path_remote}
 #rsync -rvaR ${path_zip} ${sshname}:${path_remote}
 
 # 
ssh "${sshname}" << eeooff

if [ ! -f "${path_remote_zip}"  ]; then
   echo "exe not found: ${path_remote_zip}"
   exit 1
fi

unzip -oq ${path_remote_zip} -d ${path_remote}/

if [ ! -d "${path_remote}/logs/" ]; then
    echo "remote mkdir ${path_remote}/logs/ "
    mkdir -p ${path_remote}/logs/
fi

if [ ! -f "/etc/supervisor/conf.d/${file_supervisor}"  ]; then
   sudo ln -s -b ${path_remote_supervisor_conf} /etc/supervisor/conf.d
fi

supervisorctl update
#supervisorctl start ${supervisorctl_pragram}

#xxx=$(supervisorctl status | grep -Po ${supervisorctl_pragram})
echo "xxx:`supervisorctl status | grep -Po ${supervisorctl_pragram}`"

if [ -z "`supervisorctl status | grep -Po ${supervisorctl_pragram}`" ]; then
   echo " supervisorctl status: ${supervisorctl_pragram} null"
   exit 1
fi

supervisorctl start "`supervisorctl status | grep -Po ${supervisorctl_pragram}`"

echo "ok"
exit
eeooff

 # 
 #echo "---------------------------------uploader_ end"
}

# ---------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------
function execute_(){
 
 #
 exe_patch="${path_remote}/demo"
 echo "exe_patch: ${exe_patch}"
 
 # ssh "${sshname}" "exec nohup ${path_remote}/demo > ${path_remote}/logs/nohup.log 2>&1 &"
 # ssh "${sshname}" > /dev/null 2>&1 << eeooff

# > /dev/null 2>&1 加上这个的话就不会输出信息了
ssh "${sshname}" << eeooff

echo "remote ${sshname} shell begin"

if [ ! -f "${exe_patch}"  ]; then
   echo "exe not found: ${exe_patch}"
   exit 1
fi

if [ ! -d "${path_remote}/logs/" ]; then
    echo "remote mkdir ${path_remote}/logs/ "
    mkdir -p ${path_remote}/logs/
fi

xxx=$(supervisorctl status | grep -Po ${supervisorctl_pragram})

supervisorctl start ${xxx}

#nohup ${exe_patch} > ${path_remote}/logs/nohup.log 2>&1 &

echo "ok"
exit
eeooff
}

# ==========================================================================
# call
# ==========================================================================
pack_
uploader_
#execute_
#echo "ok"


