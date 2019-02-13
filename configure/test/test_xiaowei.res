Section-------[GLOBAL]
    Array--------[server] : {
        Section-------[GLOBAL.@server]
            =[ip], _value=[127.0.0.1]
            =[port], _value=[5123]
        Section-------[GLOBAL.@server]
            =[ip], _value=[127.0.0.2]
            =[port], _value=[1123]
        Section-------[GLOBAL.@server]
            =[ip], _value=[138.3.1.2]
            =[port], _value=[1234]
    }
    Section-------[GLOBAL.person]
        =[name], _value=[xiaowei]
        =[age], _value=[1]
conf size = 2
127.0.0.1
5123
127.0.0.2
1123
138.3.1.2
1234
-------catch NoSuchKey---------
