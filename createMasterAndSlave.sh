#!/bin/bash

# while IFS='=' read key value;do
#     if [[ $key != *"#"* ]];then
#         value=$(eval echo $value)
#         eval ${key}=\${value}
#     fi
# done < default.conf

chmod +x default.conf
. ./default.conf

while IFS='=' read -r key value; do
    if [[ $key != *"#"* ]];then
        eval ${key}=\${value}
    fi
done < "$property_file"

if [ -z "$pg_path" ]
then
    pg_path=$default_pg_path
fi


if [ -z "$master_data_path" ]
then
    master_data_path=$default_master_data_path
fi


if [ -z "$slave_data_path" ]
then
    slave_data_path=$default_slave_data_path
fi


if [ -z "$master_port" ]
then
    master_port=$default_master_port
fi


if [ -z "$slave_port" ]
then
    slave_port=$default_slave_port
fi

if [[ ! -d "$pg_path/pg11" ]];then
    curl ${url} --output $pg_path/$file_name
    cd $pg_path
    tar -xf postgresql-11.4.tar.gz
    cd postgresql-11.4
    ./configure --prefix="$pg_path/pg11" --without-readline --without-zlib
    make
    make install
fi

if [[ -d $master_data_path/pgdata ]];then
    rm -r $master_data_path/pgdata
fi

mkdir -p $master_data_path/pgdata

if [[ -d $slave_data_path/pgdata ]];then
    rm -r $slave_data_path/pgdata
fi

mkdir -p $slave_data_path/pgdata
# sudo chown -R $USER $master_data_path
# sudo chown -R $USER $slave_data_path

INITDB=$pg_path/pg11/bin/initdb
PG_CTL=$pg_path/pg11/bin/pg_ctl
PSQL=$pg_path/pg11/bin/psql
PG_BASEBACKUP=$pg_path/pg11/bin/pg_basebackup

$INITDB -D "$master_data_path/pgdata"

master_postgresql_conf_file=$master_data_path/pgdata/postgresql.conf
master_pg_hba_conf_file=$master_data_path/pgdata/pg_hba.conf
slave_postgresql_conf_file=$slave_data_path/pgdata/postgresql.conf


while IFS='=' read key value;do
    sed -i /"$key = "/d  $master_postgresql_conf_file 
    echo "$key = $value" >> $master_postgresql_conf_file
done < $master_conf_file

$PG_CTL -D $master_data_path/pgdata start
$PSQL postgres -c "CREATE ROLE replication WITH REPLICATION LOGIN;"
$PG_CTL -D $master_data_path/pgdata stop

mkdir -p $pg_path/pg11/archive
# sudo chown $USER $pg_path/pg11/archive
$PG_CTL -D $master_data_path/pgdata start
rm -rf $slave_data_path/pgdata/*

$PG_BASEBACKUP -h localhost -D $slave_data_path/pgdata -P -U replication

while IFS='=' read key value;do
    sed -i /"$key = "/d  $slave_postgresql_conf_file 
    echo "$key = $value" >> $slave_postgresql_conf_file
done < $slave_conf_file

touch $slave_data_path/pgdata/recovery.conf
chown -R $USER $slave_data_path
chmod 700 -R $slave_data_path/pgdata

while IFS='=' read key value;do
    sed -i /"$key = "/d  $slave_data_path/pgdata/recovery.conf
    echo "$key = $value" >> $slave_data_path/pgdata/recovery.conf
done < $recovery_conf_file

$PG_CTL -D $slave_data_path/pgdata -o "-p $slave_port" start
