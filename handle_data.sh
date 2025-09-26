#!/bin/bash

# Fail on error
set -e


if [ -z "$1" ]
then
    echo "Usage: $0 <DATE>"
    exit 1
fi

TARGET_DATE=$1
CURRENT_DIR=$(pwd)
ELECTRIX_API_DIR=${CURRENT_DIR}/electrix-api

# Load environment variables from .env file
set -o allexport
source .env
set +o allexport

if [ ! -d ${ELECTRIX_API_DIR} ]
then
    echo "Error: electrix-api directory not found in ${CURRENT_DIR}"
    exit 1
fi

# Fetch data from API and generate CSV files
mkdir -p ${LOCAL_CSV_PATH}
cd ${ELECTRIX_API_DIR}
bash run_app.sh --local ${TARGET_DATE} ${LOCAL_CSV_PATH}

# Copy CSV files to remote server
scp \
    -i ${LOCAL_SSH_PRIVATE_KEY_PATH} \
    -P ${SERVER_PORT} \
    ${LOCAL_CSV_PATH}/${TARGET_DATE}_*.csv \
    ${SERVER_USERNAME}@${SERVER_IP}:${SERVER_CSV_PATH}

# Run data transformation on remote server
ssh \
    -i ${LOCAL_SSH_PRIVATE_KEY_PATH} \
    -p ${SERVER_PORT} \
    ${SERVER_USERNAME}@${SERVER_IP} \
    "cd ${SERVER_DATA_TRANSFORMER_PATH} && SPARK_HOME=${SERVER_SPARK_HOME} ./run_app.sh ${TARGET_DATE} ${SERVER_CSV_PATH} ${SERVER_DELTA_PATH}"

# Remove local CSV files
rm ${LOCAL_CSV_PATH}/${TARGET_DATE}_*.csv

cd ${CURRENT_DIR}
