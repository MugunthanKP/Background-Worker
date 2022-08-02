#/bin/bash

DIRTOBIN=/home/local/ZOHOCORP/muguntha-pt5620/Desktop/BGWorker_04/pg11/bin

while [[ true ]];do
   
   /bin/python /home/local/ZOHOCORP/muguntha-pt5620/Desktop/BGWorker_04/insertMarks.py

    echo 'Inserted and waiting for truncating'

   sleep 10

   # $DIRTOBIN/psql postgres -c "TRUNCATE Marks" 

done
