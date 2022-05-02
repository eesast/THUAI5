#! /bin/bash
cd /usr/local/Server
./Server --port 7777 --teamCount 2 --playerCount 4 --mapResource /usr/local/$MAP --gameTimeInSecond 600 --fileName /usr/local/playback/video --resultFileName /usr/local/playback/result >/usr/local/playback/server_log.txt 2>&1 &
server_pid=$!
sleep 5
cd /usr/local/team1
i=1
while (( $i <= 4))
do
    ./capi$i -t 0 -p $[$i-1] -P 7777 -I 127.0.0.1 >/usr/local/playback/team1_log$i.txt 2>&1 &
    let "i++"
done
cd /usr/local/team2
j=1
while (( $j <= 4))
do
    ./capi$j -t 1 -p $[$j-1] -P 7777 -I 127.0.0.1 >/usr/local/playback/team2_log$j.txt 2>&1 &
    let "j++"
done
ps -p $server_pid
while [ $? -eq 0 ]
do
    sleep 1
    ps -p $server_pid
done
parse_json() {
    echo "${1//\"/}" | sed "s/.*$2:\([^,}]*\).*/\1/"
}
result=$(cat /usr/local/playback/result.json)
score0=$(parse_json $result "Team0")
score1=$(parse_json $result "Team1")
curl $URL -X PUT -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" -d '{"result":[{"team_id":0, "score":'${score0}'}, {"team_id":1, "score":'${score1}'}], "mode":'${MODE}'}'