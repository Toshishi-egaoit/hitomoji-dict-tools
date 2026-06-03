#!/bin/bash
# kdic2.small.xml から kdic2 フェーズ用の SQL を生成する。

if [ "$1" = "" ] ; then
	echo "usage: make-kdic2-sql.sh source-XML" >&2
	echo "   ex: make-kdic2-sql.sh g9.xml" >&2
	exit 1
fi

SOURCE=$1

xmlstarlet sel -t \
	-m '//character' \
	-m 'reading' \
		-v "../literal" -o "," \
		-v ../misc/grade  -o "," \
		-v ../misc/freq  -o "," \
		-v '../radical/rad_value[@rad_type="classical"]' -o "," \
		-v '../misc/stroke_count[1]' -o "," \
		-v '../codepoint/cp_value[@cp_type="ucs"]'  -o "," \
		-v '../codepoint/cp_value[@cp_type="jis208"]' -o "," \
		-v '../codepoint/cp_value[@cp_type="jis213"]' -o "," \
		-v 'substring-after(@r_type,"_")'  -o "," \
		-v "."   -n \
	"$SOURCE" > "$SOURCE.raw.csv"

awk -F, '
function jis_level(jis208, jis213, ku, ten, a) {
	if (jis208 != "") {
		split(jis208, a, "-")
		if (a[3] != "") {
			ku = a[2] + 0
			ten = a[3] + 0
		} else {
			ku = a[1] + 0
			ten = a[2] + 0
		}
		if ((ku >= 16 && ku <= 46) || (ku == 47 && ten <= 51)) {
			return 1
		}
		return 2
	}
	if (jis213 ~ /^1-/) {
		return 3
	}
	if (jis213 ~ /^2-/) {
		return 4
	}
	return "NULL"
}
{
	print $1 "," $2 "," $3 "," $4 "," $5 "," jis_level($7, $8) "," $6 "," $9 "," $10
}
' "$SOURCE.raw.csv" > "$SOURCE.csv"

xmlstarlet sel -t \
	-m '//character/nanori' \
		-v '../codepoint/cp_value[@cp_type="ucs"]' -o "," \
		-v "." -n \
	"$SOURCE" > "$SOURCE.nanori.csv"

sed \
	-e 's/^/FUNC(/' \
	-e 's/$/)/' \
	-e 's/\./,/' \
	-e 's/-//' \
	-e 'y/アイウエオカキクケコサシスセソ/あいうえおかきくけこさしすせそ/' \
	-e 'y/タチツテトナニヌネノハヒフヘホ/たちつてとなにぬねのはひふへほ/' \
	-e 'y/マミムメモヤユヨラリルレロ/まみむめもやゆよらりるれろ/' \
	-e 'y/ワヲンッャュョ/わをんっゃゅょ/' \
	-e 'y/ガギグゲゴザジズゼゾダヂヅデド/がぎぐげござじずぜぞだぢづでど/' \
	-e 'y/バビブベボパピプペポ/ばびぶべぼぱぴぷぺぽ/' \
	< "$SOURCE.csv" > "$SOURCE.m4"

cat <<'EEOOFF' > macro_k.m4
# table k insertion (#1=literal, #2=grade, #3=freq, #4=radical, #5=strokes, #6=jis_level, #7=codepoint)
define(GRADE, `ifelse($1,,0,$1)')
define(FREQ, `ifelse($1,,3000,$1)')
define(FUNC, insert into k (letter, grade, freq, radical, strokes, jis_level, cp) values ('$1', `GRADE($2)', `FREQ($3)', $4, $5, $6, 0x$7);)
EEOOFF

m4 macro_k.m4 "$SOURCE.m4" | uniq > "$SOURCE.k.sql"

cat <<'EEOOFF' > macro_y.m4
# table y_base insertion ($7=codepoint, $8=type, $9=yomi, $10=okuri)
define(FUNC, insert into y_base (cp, tp, yomi, okuri, fromKdic) values (0x$7, '$8', '$9', '$10', 1);)
EEOOFF

m4 macro_y.m4 "$SOURCE.m4" > "$SOURCE.y.sql"

sed \
	-e 's/^/FUNC(/' \
	-e 's/$/)/' \
	-e 'y/アイウエオカキクケコサシスセソ/あいうえおかきくけこさしすせそ/' \
	-e 'y/タチツテトナニヌネノハヒフヘホ/たちつてとなにぬねのはひふへほ/' \
	-e 'y/マミムメモヤユヨラリルレロ/まみむめもやゆよらりるれろ/' \
	-e 'y/ワヲンッャュョ/わをんっゃゅょ/' \
	-e 'y/ガギグゲゴザジズゼゾダヂヅデド/がぎぐげござじずぜぞだぢづでど/' \
	-e 'y/バビブベボパピプペポ/ばびぶべぼぱぴぷぺぽ/' \
	< "$SOURCE.nanori.csv" > "$SOURCE.nanori.m4"

cat <<'EEOOFF' > macro_nanori.m4
# table y_nanori insertion (#1=codepoint, #2=yomi)
define(FUNC, insert or ignore into y_nanori (cp, yomi) values (0x$1, '$2');)
EEOOFF

m4 macro_nanori.m4 "$SOURCE.nanori.m4" > "$SOURCE.nanori.sql"
