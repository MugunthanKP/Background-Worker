#!/bin/bash

current_directory=$PWD
url="https://ftp.postgresql.org/pub/source/v11.4/postgresql-11.4.tar.gz"     
File_name="postgresql-11.4.tar.gz"                                          
Default_port=5432                                                           
Default_install_path=$PWD                                                   
Default_build_path=$PWD
Custom_conf_file="$current_directory/custom.conf"

file="./datas.properties"
while IFS='=' read -r key value; do
    if [[ $key != *"#"* ]];then
        eval ${key}=\${value}
    fi
done < "$file"

# InstallPathFunction

if [ -z "$install_path" ]
then
    install_path=$Default_install_path
fi

# BuildPathFunction

if [ -z "$build_path" ]
then
    build_path=$Default_build_path
fi

# PortFunction

if [ -z "$port" ]
then
    port=$Default_port
fi


curl ${url} --output $install_path/$File_name

cd $install_path

tar -xf postgresql-11.4.tar.gz

cd postgresql-11.4

./configure --prefix=$install_path/pg11 

make

make install

mkdir $build_path/pgdata

chown $USER $build_path/pgdata

cd $install_path/pg11/bin

INITDB=$install_path/pg11/bin/initdb 

$INITDB -D $build_path/pgdata

# postgresql_conf_file=$build_path/postgresql.conf

# while IFS=' = ' read -r key value
# do
#     if grep -q "$key = " "$postgresql_conf_file";then
#         export oldValue=$(grep "$key = " "$postgresql_conf_file" | cut -d'=' -f2)
#         sed -i "s/$oldValue/"$value"/g" "$postgresql_conf_file"        
#     else
#         echo "Not found $key"
#         echo "$key = $value" >> "$postgresql_conf_file"
#     fi
# done < "$Custom_conf_file"

printf "\n*************************************************************************************************\n"
