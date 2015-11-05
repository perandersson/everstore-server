set -o nounset
set -o errexit

echo "Building everstore image..."
docker build -t everstore:latest https://raw.githubusercontent.com/perandersson/everstore/master/Dockerfile

## echo "Flattening image"
## docker run --name everstore-big everstore
## docker export $(docker ps -a | grep everstore-big |awk '{print $1}') | docker import - everstore-small

