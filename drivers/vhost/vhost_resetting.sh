SOURCE_DIR=$HOME/Documents/nfs

KERNEL_DIR=$SOURCE_DIR/linux-source-4.4.0-on2
TARGET_DIR=$SOURCE_DIR/vhost_after

net_dir=$KERNEL_DIR/drivers/net
vhost_dir=$KERNEL_DIR/drivers/vhost
KDIR=/lib/modules/4.4.162/build

#cd $net_dir
#make -C $KDIR M=$PWD clean
#make -C $KDIR M=$PWD modules
#make -C $KDIR M=$PWD modules_install

cd $vhost_dir
make -C $KDIR M=$PWD clean
make -C $KDIR M=$PWD modules

cp $net_dir/tun.ko         	$TARGET_DIR
cp $net_dir/macvlan.ko     	$TARGET_DIR
cp $net_dir/macvtap.ko     	$TARGET_DIR
cp $vhost_dir/cuju_module.ko	$TARGET_DIR
cp $vhost_dir/vhost.ko     	$TARGET_DIR
cp $vhost_dir/vhost_net.ko 	$TARGET_DIR
