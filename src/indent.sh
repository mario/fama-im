for i in `find ./ -name "*.[hc]"`; do
	echo $i;
	indent -br -brs -bad -bap -bbb -ncs -ce -npcs -nbbo -cdb -sc -i8 -l80 "$i"; 
done;

for i in `find ./ -name "*~"`; do
	rm "$i"
done
