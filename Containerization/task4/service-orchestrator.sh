#!/bin/bash

# Complete this script to deploy external-service and counter-service in two separate containers
# You will be using the conductor tool that you completed in task 3.

# Creating link to the tool within this directory
ln -s ../task3/conductor.sh conductor.sh
ln -s ../task3/setup.sh setup.sh

# use the above scripts to accomplish the following actions -

# Logical actions to do:
# 1. Build images for the containers
./conductor.sh build cs-image csfile
./conductor.sh build es-image esfile

# 2. Run two containers say es-cont and cs-cont which should run in background. Tip: to keep the container running
#    in background you should use a init program that will not interact with the terminal and will not
#    exit. e.g. sleep infinity, tail -f /dev/null

# ./conductor.sh stop es-cont
# ./conductor.sh stop cs-cont

./conductor.sh run cs-image cs-cont sleep infinity &
./conductor.sh run es-image es-cont sleep infinity &
sleep 2

# 3. Configure network such that:
#    3.a: es-cont is connected to the internet and es-cont has its port 8080 forwarded to port 3000 of the host
./conductor.sh addnetwork es-cont -i -e 8080-3000

#    3.b: cs-cont is connected to the internet and does not have any port exposed
./conductor.sh addnetwork cs-cont -i

#    3.c: peer network is setup between es-cont and cs-cont
./conductor.sh peer cs-cont es-cont

# 5. Get ip address of cs-cont. You should use script to get the ip address. 
#    You can use ip interface configuration within the host to get ip address of cs-cont or you can 
#    exec any command within cs-cont to get it's ip address

ip_address=$(./conductor.sh exec cs-cont ip addr show | grep "cs-cont" | grep "inet" | awk '{print $2}' | cut -d/ -f1)
# echo "ip address: $ip_address"

# 6. Within cs-cont launch the counter service using 
#    exec cs-cont -- [path to counter-service directory within cs-cont]/counter-service 8080 1
./conductor.sh exec cs-cont /counter-service/counter-service 8080 1 &

# 7. Within es-cont launch the external service using 
#    exec es-cont -- python3 [path to external-service directory within es-cont]/app.py "http://<ipaddr_cs-cont>:8080/"
./conductor.sh exec es-cont python3 /external-service/app.py "http://$ip_address:8080/" &
# ------------------------------------------------------------
# bash script exiting before servers could start in background.
# That is why I added sleep explicitly of 2 seconds
sleep 2
# ------------------------------------------------------------
# 8. Within your host system open/curl the url: http://localhost:3000 to verify output of the service
# 9. On any system which can ping the host system open/curl the url: `http://<host-ip>:3000` to verify
#    output of the service  