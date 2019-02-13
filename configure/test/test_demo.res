Module Name : Config Demo
We have 2 servers.
Server[0] is : ip = 127.0.0.1, port = 9900
Server[1] is : ip = 192.168.1.2, port = 9901

-------dump-------
module : Config Demo
[@server]
ip : 127.0.0.1
port : 9900
$include : inc.conf
[@server]
ip : 192.168.1.2
port : 9901

[GLOBAL]
#this is inc
name : inc
age : 12
$include : "Hello"

--------print---------
Section-------[GLOBAL]
    =[module], _value=[Config Demo]
    =[name], _value=[inc]
    =[age], _value=[12]
    Array--------[server] : {
        Section-------[GLOBAL.@server]
            =[ip], _value=[127.0.0.1]
            =[port], _value=[9900]
        Section-------[GLOBAL.@server]
            =[ip], _value=[192.168.1.2]
            =[port], _value=[9901]
    }
----nxt.print----
Section-------[GLOBAL]
    =[module], _value=[Config Demo]
    =[name], _value=[inc]
    =[age], _value=[12]
    Array--------[server] : {
        Section-------[GLOBAL.@server]
            =[ip], _value=[127.0.0.1]
            =[port], _value=[9900]
        Section-------[GLOBAL.@server]
            =[ip], _value=[192.168.1.2]
            =[port], _value=[9901]
    }
