#!/bin/bash

#install vim
[ -z `which vim` ] && sudo apt install -y vim
#install tmux
[ -z `which tmux` ] && sudo apt install -y tmux
#install git
[ -z `which git` ] && sudo apt install -y git
#install spihx
if [ -z `which sphinx-build` ];then
	sudo apt install -y python3-pip
	pip3 install sphinx
    pip3 install sphinx_rtd_theme
fi
#install vimplus
vimplus_giturl=https://github.com/hkdywg/vimplus.git
git clone ${vimplus_giturl} ~/.vim
cd ~/.vim
ln -s ~/.vim/.vimrc ~/.vimrc
cd -
#install and config apache2
if [ -z `which apache2` ];then
    sudo apt install apache2
fi


################################################
############  debug tools ######################
################################################
#network config
[ -z `which ifconfig` ] && sudo apt install -y net-tools
#ssh install and start
if [ -z `which sshd` ];then
	sudo apt install -y openssh-server
	sudo /etc/init.d/ssh start
fi
#install tftp
if [ -z `which tftp` ];then
	TFTP_PATH=/home/tftp
	sudo apt install -y tftp-hpa tftpd-hpa
	sudo mkdir -p ${TFTP_PATH}
	sudo sed -i 's/TFTP_DIRECTORY="\/var\/lib\/tftpboot"/TFTP_DIRECTORY="\/home\/tftp"/' /etc/default/tftpd-hpa
fi
#install nfs server
if [ ! -d /home/nfs ];then
	sudo apt install -y nfs-kernel-server
	sudo mkdir -p /home/nfs
	#sudo echo "/home/nfs *(rw,sync,no_root_squash,no_subtree_check)" >> /etc/exports
	last_line=`wc  -l /etc/exports | awk '{print $1}'`	
	sudo sed -i "${last_line}a\/home\/nfs *(rw,sync,no_root_squash,no_subtree_check)" /etc/exports
	sudo /etc/init.d/rpcbind restart
	sudo /etc/init.d/nfs-kernel-server restart
fi
#install htop
[ -z `which htop` ] && sudo apt install -y htop



#install minicom
[ -z `which minicom` ] && sudo apt install -y minicom



#install u-boot and kernel tools
sudo apt install -y build-essential autoconf automake bison flex libssl-dev bc u-boot-tools repo
[ -z `which cmake` ] && sudo apt install -y cmake


#install kernel tools
sudo apt install -y device-tree-compiler python-pip ncurses-dev binfmt-support libssl-dev liblz4-tool
#install zlib...
sudo apt-get install -y zlib1g-dev flex python-numpy android-tools-fsutils mtd-utils
