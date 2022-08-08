-- complain if script is sourced in psql, rather than via CREATE EXTENSION
-- \echo Use "CREATE EXTENSION bw_sm" to load this file. \quit
-- CREATE FUNCTION launch_insert_sm()
--     RETURNS pg_catalog.int4 STRICT
-- 	AS 'MODULE_PATHNAME' LANGUAGE C;
-- REVOKE ALL ON FUNCTION launch_insert_sm()
-- 	FROM public;


\echo Use "CREATE EXTENSION bw_sm" to load this file. \quit
CREATE FUNCTION launch_insert_sm()
    RETURNS cstring STRICT
	AS 'MODULE_PATHNAME' LANGUAGE C;

CREATE FUNCTION launch_insert_sm_util()
    RETURNS TEXT STRICT
	LANGUAGE plpgsql
	AS $$
	DECLARE
		is_in_recovery VARCHAR(10);
		result VARCHAR(100);
	BEGIN
		
		SELECT pg_is_in_recovery() INTO is_in_recovery;
		IF is_in_recovery = 'true' THEN
			result := 'pg is in recovery u can not  start background writer';
		ELSE
			SELECT launch_insert_sm() INTO result;
		END IF;
		RETURN result;
	END;
	$$;


REVOKE ALL ON FUNCTION launch_insert_sm()  FROM CURRENT_USER;