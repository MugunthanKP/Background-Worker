-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION bwstudentsmarkstoppers" to load this file. \quit

CREATE TABLE IF NOT EXISTS students
	(student_id int,
	name VARCHAR(50),
	address VARCHAR(50),
	dob date,
	contact VARCHAR(50)
	);
CREATE TABLE IF NOT EXISTS marks
	(
	student_id int,
	english int,
	maths int,
	science int,
	social int,
	gk int,
	time timestamp
	);
CREATE TABLE IF NOT EXISTS toppers
	(
	name varchar(50),
	student_id int,
	score int,
	time timestamp
	);

do $$
begin
   for i in 1..100 loop
		INSERT INTO students VALUES (i,'name'||i,'address'||i,TO_DATE('20010101','YYYYMMDD') + (random() * (interval '365 days')),cast(floor(random()*(9999999999-6000000000+1)+6000000000)as varchar));
   end loop;
end; $$

