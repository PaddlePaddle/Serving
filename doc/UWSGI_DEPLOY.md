# Deploy HTTP service with uWSGI

([简体中文](./UWSGI_DEPLOY_CN.md)|English)

In fit_a_line example, after starting the HTTP prediction service, you will see the following information:

```shell
web service address:
http://10.127.3.150:9393/uci/prediction
 * Serving Flask app "serve" (lazy loading)
 * Environment: production
   WARNING: This is a development server. Do not use it in a production deployment.
   Use a production WSGI server instead.
 * Debug mode: off
 * Running on http://0.0.0.0:9393/ (Press CTRL+C to quit)
```

Here you will be prompted that the HTTP service started is in development mode and cannot be used for production deployment. 
The prediction service started by Flask is not stable enough to withstand the concurrency of a large number of requests. In the actual deployment process, WSGI (Web Server Gateway Interface) is used.

Next, we will show how to use the [uWSGI](https://github.com/unbit/uwsgi) module to deploy HTTP prediction services for production environments.


```python
#uwsgi_service.py
from paddle_serving_server.web_service import WebService

#Define prediction service
uci_service = WebService(name = "uci")
uci_service.load_model_config("./uci_housing_model")
uci_service.prepare_server(workdir="./workdir", port=int(9500), device="cpu")
uci_service.run_rpc_service()
#Get flask application
app_instance = uci_service.get_app_instance()
```

Start service with uWSGI

```bash
uwsgi --http :9393 --module uwsgi_service:app_instance
```

Use the --processes parameter to specify the number of service processes. 

For more information about uWSGI, please refer to [uWSGI documentation](https://uwsgi-docs.readthedocs.io/en/latest/)
