create table jm_k (
	ent_seq integer,
	keb text
);

create table jm_r (
	ent_seq integer,
	reb text,
	re_nokanji integer
);

create table jm_restr (
	ent_seq integer,
	reb text,
	keb text
);

create index idx_jm_k_ent_seq on jm_k(ent_seq);
create index idx_jm_k_keb on jm_k(keb);
create index idx_jm_r_ent_seq on jm_r(ent_seq);
create index idx_jm_r_reb on jm_r(reb);
create index idx_jm_restr_ent_seq_reb on jm_restr(ent_seq, reb);
create index idx_jm_restr_keb on jm_restr(keb);

create view jm_pair as
	select k.ent_seq, k.keb, r.reb
	from jm_k k
	  inner join jm_r r on r.ent_seq = k.ent_seq
	where r.re_nokanji = 0
	  and not exists (
		select 1
		from jm_restr x
		where x.ent_seq = r.ent_seq
		  and x.reb = r.reb
	  )
	union
	select ent_seq, keb, reb
	from jm_restr;
