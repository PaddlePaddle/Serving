ps -ef | grep cube | awk {'print $2'} | xargs kill -9
rm -rf cube/cube_data cube/data cube/log* cube/nohup* cube/output/ cube/donefile cube/input cube/monitor cube/cube-builder.INFO
ps -ef | grep test | awk {'print $2'} | xargs kill -9
ps -ef | grep serving | awk {'print $2'} | xargs kill -9
