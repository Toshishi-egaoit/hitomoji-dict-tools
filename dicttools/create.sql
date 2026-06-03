
# k table
create table  k (
	letter	varchar(2) ,
	cp	integer primary key,
	grade	integer ,
	freq	integer ,
	radical	integer,
	strokes	integer,
	jis_level	integer,
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

create table y_nanori (
	cp	integer,
	yomi	varchar(10),
	foreign key(cp) references k(cp),
	unique(cp, yomi)
);

create table y_exclude (
	cp	integer,
	tp	varchar(3),
	yomi	varchar(10),
	reason	text,
	foreign key(cp) references k(cp),
	unique(cp, tp, yomi, reason)
);

create view y as
	select distinct letter, k.cp ,tp , yomi
	from y_base
	  inner join k on k.cp = y_base.cp
	where not exists (
		select 1 from y_exclude
		where y_exclude.cp = y_base.cp
		  and y_exclude.tp = y_base.tp
		  and y_exclude.yomi = y_base.yomi
	);
