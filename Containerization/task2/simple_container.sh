#!/bin/bash

SIMPLE_CONTAINER_ROOT=container_root

mkdir -p $SIMPLE_CONTAINER_ROOT

gcc -o container_prog container_prog.c

## Subtask 1: Execute in a new root filesystem

cp container_prog $SIMPLE_CONTAINER_ROOT/

# 1.1: Copy any required libraries to execute container_prog to the new root container filesystem 
# mkdir -p $SIMPLE_CONTAINER_ROOT/{lib,lib64}
dependencyList="$(ldd ./container_prog | egrep -o '/lib.*\.[0-9]')"
for i in $dependencyList; do 
    cp --parents "$i" "${SIMPLE_CONTAINER_ROOT}";
done

echo -e "\n\e[1;32mOutput Subtask 2a\e[0m"
# 1.2: Execute container_prog in the new root filesystem using chroot. You should pass "subtask1" as an argument to container_prog
sudo chroot $SIMPLE_CONTAINER_ROOT /container_prog subtask1

echo "__________________________________________"
echo -e "\n\e[1;32mOutput Subtask 2b\e[0m"
## Subtask 2: Execute in a new root filesystem with new PID and UTS namespace
# The pid of container_prog process should be 1
# You should pass "subtask2" as an argument to container_prog

sudo unshare -f -p -u chroot $SIMPLE_CONTAINER_ROOT /container_prog subtask2


echo -e "\nHostname in the host: $(hostname)"

## Subtask 3: Execute in a new root filesystem with new PID, UTS and IPC namespace + Resource Control
# Create a new cgroup and set the max CPU utilization to 50% of the host CPU. (Consider only 1 CPU core)

sudo mkdir /sys/fs/cgroup/mycgroup
sudo echo "500000 1000000" > /sys/fs/cgroup/mycgroup/cpu.max


echo "__________________________________________"
echo -e "\n\e[1;32mOutput Subtask 2c\e[0m"
# Assign pid to the cgroup such that the container_prog runs in the cgroup
# Run the container_prog in the new root filesystem with new PID, UTS and IPC namespace
# You should pass "subtask1" as an argument to container_prog

sudo echo $$ > /sys/fs/cgroup/mycgroup/cgroup.procs
sudo unshare -f -p -u -i chroot $SIMPLE_CONTAINER_ROOT /container_prog subtask3

# Remove the cgroup
sudo echo $$ > /sys/fs/cgroup/cgroup.procs
sudo rmdir /sys/fs/cgroup/mycgroup

# If mounted dependent libraries, unmount them, else ignore
