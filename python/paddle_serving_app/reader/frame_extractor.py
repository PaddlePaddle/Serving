"""
frame extractor
"""
import sys
import numpy as np
from paddle_serving_client import Client

import logging
import hwextract as hwextract
from multiprocessing.pool import ThreadPool

_LOGGER = logging.getLogger(__name__)
class FrameData(object):
    """
    store frame data in python
    """
    def __init__(self):
        """
        constructor
        """
        self._width = 0
        self._height = 0
        self._img = None

    def width(self):
        """
        return width
        """
        return self._width

    def height(self):
        """
        return height
        """
        return self._height
    
    def img(self):
        """
        return img array
        """
        return self._img

    def set_width(self, width):
        """
        set width
        """
        self._width = width

    def set_height(self, height):
        """
        set height
        """
        self._height = height

    def set_img(self, img):
        """
        set_img
        """
        self._img = img

    def set_data(self, frame):
        """
        set all data using frame
        """
        self._width = frame.width()
        self._height = frame.height()
        self._img = np.array(frame, copy=True)

class FrameExt(object):
    """ 
    extract and call tns
    """
    def __init__(self):
        """
        constructor
        """
        
        self._handler = None
       
    def init_handler(self, card_idx):
        """
        init handler
        """
        
        self._handler = hwextract.HwExtractFrameJpeg(card_idx)
        result = self._handler.init_handler()
        return result

    def extract_frame(self, file_name):
        """
        hardware extract video
        """        
        frame_list = self._handler.extract_frame(file_name, 1)
        result_list = []
        for item in frame_list:
            tmp_frame = FrameData()
            tmp_frame.set_data(item)
            result_list.append(tmp_frame)
            item.free_memory()
        return result_list
    
    def chunks(self, lst, n):
    	"""Yield successive n-sized chunks from lst."""
    	for i in range(0, len(lst), n):
        	yield lst[i:i + n]
