-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION sqe" to load this file. \quit
CREATE FUNCTION sqegenerate()
RETURNS text
LANGUAGE plpgsql VOLATILE
  AS $$
    BEGIN
      -- INSERT INTO Sample VALUES (1,'abc');

    INSERT INTO toppers (
	  SELECT students.name,students.student_id,score 
	  FROM students,(SELECT student_id,(english+maths+science+social+gk) as score 
			FROM marks
			WHERE (english+maths+science+social+gk) in (
				SELECT MAX(english+maths+science+social+gk) as score from marks)) AS temp
	    WHERE students.student_id=temp.student_id
    );
    RETURN('inserted successfully');
    END;
  $$;