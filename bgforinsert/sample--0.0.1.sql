-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION sample" to load this file. \quit
CREATE FUNCTION launch_insert()
    RETURNS pg_catalog.int4 STRICT
	AS 'MODULE_PATHNAME' LANGUAGE C;
REVOKE ALL ON FUNCTION launch_insert()
	FROM public;