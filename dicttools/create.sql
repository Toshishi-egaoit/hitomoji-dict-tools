
# k table
create table  k (
	letter	varchar(2) ,
	cp	integer primary key,
	grade	integer ,
	freq	integer ,
	slot	integer
);

create table  y_base (
	cp	integer ,
	tp	varchar(3),
	yomi	varchar(10),
	okuri	varchar(10),
	fromKdic bool,
	foreign key(cp) references k(cp)
);

create table  y_dakuon (
	cp	integer ,
	yomi	varchar(10),
	yomi_seion varchar(10),
	foreign key(cp) references k(cp)
);

creatE TABLE y_jmdict (
	cp	integer,
	yomi	varchar(10),
	wd_cnt	integer,
	foreign key(cp) references k(cp)
);

create view y as
	select distinct letter, k.cp ,tp , yomi
	from y_base
	  inner join k on k.cp = y_base.cp
	union 
	select distinct letter, k.cp ,"on" , yomi
	from y_dakuon
	  inner join k on k.cp = y_dakuon.cp
	union 
	select distinct letter, k.cp ,"on" , yomi
	from y_jmdict
	  inner join k on k.cp = y_jmdict.cp;
