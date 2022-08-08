import psycopg2
import random

try:
    connection = psycopg2.connect(user="muguntha-pt5620",
                                  password="",
                                  host="127.0.0.1",
                                  port="5432",
                                  database="postgres")
    cursor = connection.cursor()

    for i in range(1,101):
        postgres_insert_query = """ INSERT INTO students VALUES (%s,%s,%s,%s,%s)"""
        date = str(random.randrange(1999,2004,1))+'-'+str(random.randrange(1,12,1))+'-'+str(random.randrange(1,28,1))
        phone = str(random.randrange(7777777777,9999999999,1))
        record_to_insert = (i, 'name'+str(i), 'address'+str(i),date,phone)
        cursor.execute(postgres_insert_query, record_to_insert)
        connection.commit()
    print( "Record inserted successfully into mobile table")

except (Exception, psycopg2.Error) as error:
    print("Failed to insert record into mobile table", error)

finally:
    # closing database connection.
    if connection:
        cursor.close()
        connection.close()
        print("PostgreSQL connection is closed")  