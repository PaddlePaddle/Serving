# Simple Pipeline WebService

This document will takes UCI service as an example to introduce how to use Pipeline WebService.

## Get model
```
sh get_data.sh
```

## Start server

```
python3 web_service.py &>log.txt &
```

## Http test
```
curl -X POST -k http://localhost:18082/uci/prediction -d '{"key": ["x"], "value": ["0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332"]}'
```
