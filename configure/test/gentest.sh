S=(char uchar int16 uint16 int32 uint32 int64 uint64 float double std_string)
for ((i=0;i<${#S[*]};i++)) ; do 
	echo "";
	echo '//=========  Test '${S[$i]}'============='
	echo "try{"
	echo ${S[$i]} a$i =  $1.to_${S[$i]}"();" ; 
	echo 'std::cerr<<"to '${S[$i]}' successful, value = "<<a'$i'<<std::endl;'
	echo '}catch(...){
	fprintf(stderr, "to '${S[$i]}' failed!\n");
}'
#	echo ErrCode get_${S[$i]}'('${S[$i]}' * valueBuf);';
#	echo ;
done;
echo ""
echo ""
echo ""

for ((i=0;i<${#S[*]};i++)) ; do 
#	echo ${S[$i]} to_${S[$i]}"(ErrCode * errCode = NULL);" ; 
	echo "";
	echo '//=========  Test '${S[$i]}'============='
	echo ${S[$i]} val$i';'
	echo int ret$i = $1.get_${S[$i]}'(&val'$i');';
	echo "if(ret"$i' == 0){'
	echo 'std::cerr<<"Get '${S[$i]}' successful, value = "<<val'$i'<<std::endl;'
	echo "}else{"
	echo 'fprintf(stderr, "Get '${S[$i]}' failed!\n");'
	echo "}"
#	echo ;
done;
