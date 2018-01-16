IFS=$'\n'
user=20
count=1000000000

for ((;1;));do

for api in `cat postfile/api_list`;do
	date_time=`date  '+%Y-%m-%d %H:%M:%S'`
	echo  "---------------------------------------------------------------------------------------------------------"  >> metrics.log
	echo  $date_time "start " $api  >> metrics.log
	
	postdata=`echo $api | cut -d " " -f 2`
	uri=`echo $api | cut -d " " -f 4`

	nohup ab -c $user -n $count -k -p $postdata $uri >> metrics.log &
	sleep 600
	kill  -2 $!
	
	if [ $? -ne 0 ]; then
		date_time=`date  '+%Y-%m-%d %H:%M:%S'`
		echo $date_time "end " $api  >> metrics.log
		exit 1
	fi;

	date_time=`date  '+%Y-%m-%d %H:%M:%S'`
	echo $date_time "end " $api  >> metrics.log
done

done
