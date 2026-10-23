#!/bin/sh

set -euo pipefail
cd "$(dirname "$0")"

start_dir="$PWD"

mvn clean package
java -jar target/filemagik-1.0.jar "${1:-$start_dir}"
