echo -e "\r\n\r\n/hdp/kvstore/kv/set-------------------------------------------------------------"
curl  -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"k\":\"qkdtestkey\",\"v\":\"123\"}"  "10.10.105.61/hdp/kvstore/kv/set"


echo -e "\r\n\r\n/hdp/kvstore/kv/get-----------------------------------------------------------\r\n"
curl  -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"k\":\"qkdtestkey\"}"  "10.10.105.61/hdp/kvstore/kv/get"


echo -e "\r\n\r\n/hdp/kvstore/kv/ttl-----------------------------------------------------------\r\n"
curl  -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"k\":\"qkdtestkey\"}"  "10.10.105.61/hdp/kvstore/kv/ttl"


echo -e "\r\n\r\n/hdp/kvstore/kv/incr-----------------------------------------------------------\r\n"
curl  -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"k\":\"qkdtestkey\"}"  "10.10.105.61/hdp/kvstore/kv/incr"


echo -e "\r\n\r\n/hdp/kvstore/kv/incrby-----------------------------------------------------------\r\n"
curl  -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"k\":\"qkdtestkey\",\"v\":\"10\"}"  "10.10.105.61/hdp/kvstore/kv/incrby"


echo -e "\r\n\r\n/hdp/kvstore/kv/del-----------------------------------------------------------\r\n"
curl  -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"k\":\"qkdtestkey\"}"  "10.10.105.61/hdp/kvstore/kv/del"


echo -e "\r\n\r\n/hdp/kvstore/kv/mset-----------------------------------------------------------\r\n"
curl  -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"ks\":[{\"k\":\"testkey1\",\"v\":\"value1\"},{\"k\":\"testkey2\",\"v\":\"value2\"}]}"  "10.10.105.61/hdp/kvstore/kv/mset"


echo -e "\r\n\r\n/hdp/kvstore/kv/mget-----------------------------------------------------------\r\n"
curl  -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"ks\":[{\"k\":\"testkey1\"},{\"k\":\"testkey2\"}]}"  "10.10.105.61/hdp/kvstore/kv/mget"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/zfixedset/add-----------------------------------------------------------\r\n"
curl -d "{\"uri\" : \"qkd://BJTEST/Z6\",\"ak\" : \"aMRMzw7k2y\",\"k\" : \"gaosongtestzset\",\"mbs\" : [ {\"mb\" : \"gsMemberId1\",\"sc\" : 1000,\"v\" : \"memberValue1\"}, {\"mb\" : \"gsMemberId2\",\"sc\" : 2000,\"v\" : \"memberValue2\"}, {\"mb\" : \"gsMemberId3\",\"sc\" : 3000,\"v\" : \"memberValue3\"}, {\"mb\" : \"gsMemberId4\",\"sc\" : 4000,\"v\" : \"memberValue4\"}, {\"mb\" : \"gsMemberId5\",\"sc\" : 5000,\"v\" : \"memberValue5\"} ],\"mc\" : 15}" "10.10.105.61/hdp/kvstore/zfixedset/add"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/zfixedset/batchadd-----------------------------------------------------------\r\n"
curl -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"ks\":[{\"k\":\"userid1\",\"mbs\":[{\"mb\":\"userid1_vvid1\",\"sc\":1000,\"v\":\"value1\"}]},{\"k\":\"userid2\",\"mbs\":[{\"mb\":\"userid2_vvid1\",\"sc\":2000,\"v\":\"value2\"}]}]}"   "10.10.105.61/hdp/kvstore/zfixedset/batchadd"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/zfixedset/getbyscore-----------------------------------------------------------\r\n"
curl -d "{\"k\":\"userid1\",\"min\":1000,\"max\":5000,\"ws\":true,\"asc\":false,\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\"}"  "10.10.105.61/hdp/kvstore/zfixedset/getbyscore"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/zfixedset/getbyrank-----------------------------------------------------------\r\n"
curl -d "{\"k\":\"userid1\",\"min\":0,\"max\":2,\"ws\":true,\"asc\":false,\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\"}"  "10.10.105.61/hdp/kvstore/zfixedset/getbyrank"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/zfixedset/batchgetbyscore-----------------------------------------------------------\r\n"
curl -d "{\"uri\":\"qkd://BJTEST/Z6\",\"ak\":\"aMRMzw7k2y\",\"ks\":[{\"k\":\"userid1\",\"min\":0,\"max\":7000,\"ws\":true,\"asc\":false},{\"k\":\"userid2\",\"min\":0,\"max\":10000,\"ws\":true,\"asc\":false}]}"  "10.10.105.61/hdp/kvstore/zfixedset/batchgetbyscore"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/set/sadd-----------------------------------------------------------\r\n"
curl -d "{\"uri\" : \"qkd://BJTEST/Z6\",\"ak\" : \"aMRMzw7k2y\",\"k\" : \"gstestkey2\",\"mbs\" : [ \"m2\", \"m3\", \"m4\" ]}"  "10.10.105.61/hdp/kvstore/set/sadd"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/set/srem-----------------------------------------------------------\r\n"
curl -d "{\"uri\" : \"qkd://BJTEST/Z6\",\"ak\" : \"aMRMzw7k2y\",\"k\" : \"gstestkey2\",\"mbs\" : [ \"m4\" ]}"  "10.10.105.61/hdp/kvstore/set/srem"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/set/scard-----------------------------------------------------------\r\n"
curl -d "{\"uri\" : \"qkd://BJTEST/Z6\",\"ak\" : \"aMRMzw7k2y\",\"k\" : \"gstestkey2\"}"  "10.10.105.61/hdp/kvstore/set/scard"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/set/smembers-----------------------------------------------------------\r\n"
curl -d "{\"uri\" : \"qkd://BJTEST/Z6\",\"ak\" : \"aMRMzw7k2y\",\"k\" : \"gstestkey2\"}"  "10.10.105.61/hdp/kvstore/set/smembers"


echo -e "\r\n\r\n/hdp/kvstore/kvstore/set/sismembers-----------------------------------------------------------\r\n"
curl -d "{\"uri\" : \"qkd://BJTEST/Z6\",\"ak\" : \"aMRMzw7k2y\",\"k\" : \"gstestkey2\", \"v\" : \"m2\"}"  "10.10.105.61/hdp/kvstore/set/sismember"

echo -e "\r\n-----------------------------------------------------------\r\n"

