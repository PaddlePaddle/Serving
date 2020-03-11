ps -ef | grep cube | awk {'print $2'} | xargs kill -9
ps -ef | grep SimpleHTTPServer | awk {'print $2'} | xargs kill -9
rm -rf cube/cube_data cube/data cube/log cube/output cube/donefile
ps -ef | grep test | awk {'print $2'} | xargs kill -9
ps -ef | grep serving | awk {'print $2'} | xargs kill -9
