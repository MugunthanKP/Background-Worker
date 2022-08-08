import psycopg2
import random
from datetime import datetime


try:
    connection = psycopg2.connect(user="muguntha-pt5620",
                                  password="",
                                  host="127.0.0.1",
                                  port="5432",
                                  database="postgres")
    cursor = connection.cursor()

    for i in range(1,101):
        postgres_insert_query = """ INSERT INTO marks VALUES (%s,%s,%s,%s,%s,%s,%s)"""
        english = random.randrange(60,95,1)
        maths = random.randrange(60,95,1)
        science = random.randrange(60,95,1)
        social = random.randrange(60,95,1)
        gk = random.randrange(60,95,1)
        curr_time = datetime.now()
        formatted_time = curr_time.strftime('%Y-%m-%d %H:%M:%S')
        record_to_insert = (i,english,maths,science,gk,social,formatted_time)
        cursor.execute(postgres_insert_query, record_to_insert)
    connection.commit()
    count = cursor.rowcount
    print(count, "Record inserted successfully into mobile table")

except (Exception, psycopg2.Error) as error:
    print("Failed to insert record into mobile table", error)

finally:
    # closing database connection.
    if connection:
        cursor.close()
        connection.close()
        print("PostgreSQL connection is closed")  