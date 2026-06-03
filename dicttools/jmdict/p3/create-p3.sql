drop view if exists y;

create table if not exists y_p3_g1 (
	cp	integer,
	yomi	varchar(10),
	wd_cnt	integer,
	foreign key(cp) references k(cp),
	unique(cp, yomi)
);

create table if not exists y_p3_g2 (
	cp	integer,
	yomi	varchar(10),
	wd_cnt	integer,
	foreign key(cp) references k(cp),
	unique(cp, yomi)
);

create table if not exists y_p3_g3 (
	cp	integer,
	yomi	varchar(10),
	wd_cnt	integer,
	foreign key(cp) references k(cp),
	unique(cp, yomi)
);

create table if not exists y_p3_g4 (
	cp	integer,
	yomi	varchar(10),
	wd_cnt	integer,
	foreign key(cp) references k(cp),
	unique(cp, yomi)
);

create table if not exists y_p3_g5 (
	cp	integer,
	yomi	varchar(10),
	wd_cnt	integer,
	foreign key(cp) references k(cp),
	unique(cp, yomi)
);

create table if not exists y_p3_g6 (
	cp	integer,
	yomi	varchar(10),
	wd_cnt	integer,
	foreign key(cp) references k(cp),
	unique(cp, yomi)
);

create table if not exists y_p3_gjo (
	cp	integer,
	yomi	varchar(10),
	wd_cnt	integer,
	foreign key(cp) references k(cp),
	unique(cp, yomi)
);

create table if not exists y_p3_gjin (
	cp	integer,
	yomi	varchar(10),
	wd_cnt	integer,
	foreign key(cp) references k(cp),
	unique(cp, yomi)
);

create view y as
	select distinct letter, k.cp, tp, yomi
	from y_base
	  inner join k on k.cp = y_base.cp
	where not exists (
		select 1 from y_exclude
		where y_exclude.cp = y_base.cp
		  and y_exclude.tp = y_base.tp
		  and y_exclude.yomi = y_base.yomi
	)
	union
	select distinct letter, k.cp, 'on', yomi from y_g1 inner join k on k.cp = y_g1.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_g2 inner join k on k.cp = y_g2.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_g3 inner join k on k.cp = y_g3.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_g4 inner join k on k.cp = y_g4.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_g5 inner join k on k.cp = y_g5.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_g6 inner join k on k.cp = y_g6.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_gjo inner join k on k.cp = y_gjo.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_gjin inner join k on k.cp = y_gjin.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_p3_g1 inner join k on k.cp = y_p3_g1.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_p3_g2 inner join k on k.cp = y_p3_g2.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_p3_g3 inner join k on k.cp = y_p3_g3.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_p3_g4 inner join k on k.cp = y_p3_g4.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_p3_g5 inner join k on k.cp = y_p3_g5.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_p3_g6 inner join k on k.cp = y_p3_g6.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_p3_gjo inner join k on k.cp = y_p3_gjo.cp
	union
	select distinct letter, k.cp, 'on', yomi from y_p3_gjin inner join k on k.cp = y_p3_gjin.cp;
