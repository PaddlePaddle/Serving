import sys 
import enum 
import os 
import logging 
import traceback
from paddle_serving_server.pipeline import ResponseOp
import threading
import inspect
import traceback
import functools
import re
from .proto import pipeline_service_pb2_grpc, pipeline_service_pb2

_LOGGER = logging.getLogger(__name__) 

class Singleton(object):
    _lock = threading.Lock()
    def __new__(cls, *args, **kwargs):
        if not hasattr(Singleton, "_instance"):
            with Singleton._lock:
                if not hasattr(Singleton, "_instance"):
                    Singleton._instance = super(Singleton, cls).__new__(cls, *args, **kwargs)
        return Singleton._instance

    def set_exception_response(self, error_code, error_info):
        
        self.resp = pipeline_service_pb2.Response()
        self.resp.err_no = error_code
        self.resp.err_msg = error_info.replace("\n", " ").replace("\t", " ")[2:]

    def get_exception_response(self):
        if hasattr(self, "resp"):
            return self.resp
        else:
            return None

class CustomExceptionCode(enum.Enum): 
    """
    Add new Exception
    """
    INTERNAL_EXCEPTION = 0
    TYPE_EXCEPTION = 1
    TIMEOUT_EXCEPTION = 2
    CONF_EXCEPTION = 3
    PARAMETER_INVALID = 4

class CustomException(Exception):
    def __init__(self, exceptionCode, errorMsg, isSendToUser):
        super().__init__(self)
        self.error_info = "\n\texception_code: {}\n"\
                          "\texception_type: {}\n"\
                          "\terror_msg: {}"\
                          "\tis_send_to_user: {}".format(exceptionCode.value,
                          CustomExceptionCode(exceptionCode).name, errorMsg, isSendToUser)
    
    def __str__(self):
        return self.error_info

class ErrorCatch():
    def __call__(self, func):
        if inspect.isfunction(func) or inspect.ismethod(func):
            @functools.wraps(func)
            def wrapper(*args, **kw):
                try:
                    func(*args, **kw)
                except CustomException  as e:
                    _LOGGER.error("{}\tFunctionName: {}{}".format(traceback.format_exc(), func.__name__, args))
                    split_list = re.split("\n|\t|:", str(e))
                    error_code = int(split_list[3])
                    error_info = "{}\n\tClassName: {} FunctionName: {}".format(str(e), func.__class__ ,func.__name__)
                    is_send_to_user = split_list[-1]
                    if is_send_to_user == True:
                        self.record_error_info(error_code, error_info)
                    else:
                        raise("server error occur")

            return wrapper
    
    def record_error_info(self, error_code, error_info):
        ExceptionSingleton.set_exception_response(error_code, error_info)

ErrorCatch = ErrorCatch()
ExceptionSingleton = Singleton()
