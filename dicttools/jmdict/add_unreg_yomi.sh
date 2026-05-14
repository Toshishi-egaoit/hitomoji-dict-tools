#!/bin/sh

DB=../dict.db

sqlite3 "$DB" <<'EEOOFF'
delete from y_jmdict ;
begin;
.read add_unreg_yomi.sql 
commit;
select 'unreg_yomi=', count(*) from y_jmdict;
EEOOFF

